// Copyright (c) 2012-2013 Eric Lombrozo, The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletmanager.h"
#include "walletdb.h"

using namespace std;

////////////////////////////////////////////////////////////
//
// TODO: Move GetFilesAtPath to utils.h/utils.cpp
//
namespace file_option_flags
{
    const unsigned int REGULAR_FILES = 0x01;
    const unsigned int DIRECTORIES = 0x02;
};

vector<string> GetFilesAtPath(const boost::filesystem::path& _path, unsigned int flags)
{
    vector<string> vstrFiles;
    if (!boost::filesystem::exists(_path))
        throw runtime_error("Path does not exist.");
    
    if ((flags & file_option_flags::REGULAR_FILES) && boost::filesystem::is_regular_file(_path))
    {
#if defined (BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
        vstrFiles.push_back(_path.filename().string());
#else
        vstrFiles.push_back(_path.filename());
#endif
        return vstrFiles;
    }
    if (boost::filesystem::is_directory(_path))
    {
        vector<boost::filesystem::path> vPaths;
        copy(boost::filesystem::directory_iterator(_path), boost::filesystem::directory_iterator(), back_inserter(vPaths));
        BOOST_FOREACH(const boost::filesystem::path& pFile, vPaths)
        {
            if (((flags & file_option_flags::REGULAR_FILES) && boost::filesystem::is_regular_file(pFile)) ||
                ((flags & file_option_flags::DIRECTORIES) && boost::filesystem::is_directory(pFile)))
#if defined (BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
                vstrFiles.push_back(pFile.filename().string());
#else
            vstrFiles.push_back(pFile.filename());
#endif
        }
        return vstrFiles;
    }
    throw runtime_error("Path exists but is neither a regular file nor a directory.");
}
//
////////////////////////////////////////////////////////////


// TODO: Remove these functions
bool static InitError(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_ERROR);
    return false;
}

bool static InitWarning(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, "", CClientUIInterface::MSG_WARNING);
    return true;
}

// TODO: Remove dependencies for I/O on printf to debug.log, InitError, and InitWarning
// TODO: Fix error handling.
bool CWalletManager::LoadWallet(const string& strName, ostringstream& strErrors)
{
    // Check that the wallet name is valid
    if (!CWalletManager::IsValidName(strName))
    {
        strErrors << _("Wallet name may only contain letters, numbers, and underscores.");
        return false;
    }
    
    ENTER_CRITICAL_SECTION(cs_WalletManager);
    
    // Check that wallet is not already loaded
    if (wallets.count(strName) > 0)
    {
        LEAVE_CRITICAL_SECTION(cs_WalletManager);
        strErrors << _("A wallet with that name is already loaded.");
        return false;
    }
    
    // Wallet file name for wallet foo will be wallet-foo.dat
    // The empty string is reserved for the default wallet whose file is wallet.dat
    string strFile = "wallet";
    if (strName.size() > 0)
        strFile += "-" + strName;
    strFile += ".dat";
    
    printf("Loading wallet \"%s\" from %s...\n", strName.c_str(), strFile.c_str());
    int64 nStart = GetTimeMillis();
    bool fFirstRun = true;
    CWallet* pWallet;
    DBErrors nLoadWalletRet;
    
    try
    {
        pWallet = new CWallet(strFile);
        nLoadWalletRet = pWallet->LoadWallet(fFirstRun);
    }
    catch (const exception& e)
    {
        LEAVE_CRITICAL_SECTION(cs_WalletManager);
        strErrors << _("Critical error loading wallet \"") << strName << "\" " << _("from ") << strFile << ": " << e.what();
        return false;
    }
    catch (...)
    {
        LEAVE_CRITICAL_SECTION(cs_WalletManager);
        strErrors << _("Critical error loading wallet \"") << strName << "\" " << _("from ") << strFile;
        return false;
    }
    
    if (nLoadWalletRet != DB_LOAD_OK)
    {
        if (nLoadWalletRet == DB_CORRUPT)
        {
            LEAVE_CRITICAL_SECTION(cs_WalletManager);
            strErrors << _("Error loading ") << strFile << _(": Wallet corrupted") << "\n";
            delete pWallet;
            return false;
        }
        else if (nLoadWalletRet == DB_NONCRITICAL_ERROR)
        {
            string msg(_("Warning: error reading "));
            msg += strFile + _("! All keys read correctly, but transaction data"
                               " or address book entries might be missing or incorrect.");
            InitWarning(msg);
        }
        else if (nLoadWalletRet == DB_TOO_NEW)
            strErrors << _("Error loading ") << strFile << _(": Wallet requires newer version of Bitcoin") << "\n";
        else if (nLoadWalletRet == DB_NEED_REWRITE)
        {
            LEAVE_CRITICAL_SECTION(cs_WalletManager);
            strErrors << _("Wallet needed to be rewritten: restart Bitcoin to complete") << "\n";
            printf("%s", strErrors.str().c_str());
            return InitError(strErrors.str());
        }
        else
            strErrors << _("Error loading ") << strFile << "\n";
    }
    
    if (GetBoolArg("-upgradewallet", fFirstRun))
    {
        int nMaxVersion = GetArg("-upgradewallet", 0);
        if (nMaxVersion == 0) // the -upgradewallet without argument case
        {
            printf("Performing wallet upgrade to %i\n", FEATURE_LATEST);
            nMaxVersion = CLIENT_VERSION;
            pWallet->SetMinVersion(FEATURE_LATEST); // permanently upgrade the wallet immediately
        }
        else
            printf("Allowing wallet upgrade up to %i\n", nMaxVersion);
        if (nMaxVersion < pWallet->GetVersion())
            strErrors << _("Cannot downgrade wallet") << "\n";
        pWallet->SetMaxVersion(nMaxVersion);
    }
    
    if (fFirstRun)
    {
        // Create new keyUser and set as default key
        RandAddSeedPerfmon();
        
        CPubKey newDefaultKey;
        if (!pWallet->GetKeyFromPool(newDefaultKey, false))
            strErrors << _("Cannot initialize keypool") << "\n";
        pWallet->SetDefaultKey(newDefaultKey);
        if (!pWallet->SetAddressBookName(pWallet->vchDefaultKey.GetID(), ""))
            strErrors << _("Cannot write default address") << "\n";
    }
    
    printf("%s", strErrors.str().c_str());
    printf(" wallet      %15"PRI64d"ms\n", GetTimeMillis() - nStart);
    
    boost::shared_ptr<CWallet> spWallet(pWallet);
    this->wallets[strName] = spWallet;
    RegisterWallet(pWallet);
    
    LEAVE_CRITICAL_SECTION(cs_WalletManager);
    
    CBlockIndex *pindexRescan = pindexBest;
    if (GetBoolArg("-rescan"))
        pindexRescan = pindexGenesisBlock;
    else
    {
        CWalletDB walletdb(strFile);
        CBlockLocator locator;
        if (walletdb.ReadBestBlock(locator))
            pindexRescan = locator.GetBlockIndex();
    }
    if (pindexBest && pindexBest != pindexRescan)
    {
        uiInterface.InitMessage(_("Rescanning..."));
        printf("Rescanning last %i blocks (from block %i)...\n", pindexBest->nHeight - pindexRescan->nHeight, pindexRescan->nHeight);
        nStart = GetTimeMillis();
        pWallet->ScanForWalletTransactions(pindexRescan, true);
        printf(" rescan      %15"PRI64d"ms\n", GetTimeMillis() - nStart);
    }
    
    return true;
}

bool CWalletManager::UnloadWallet(const std::string& strName)
{
    {
        LOCK(cs_WalletManager);
        if (!wallets.count(strName)) return false;
        boost::shared_ptr<CWallet> spWallet(wallets[strName]);
        printf("Unloading wallet %s\n", strName.c_str());
        {
            LOCK(spWallet->cs_wallet);
            UnregisterWallet(spWallet.get());
            wallets.erase(strName);
        }
    }
    return true;
}

void CWalletManager::UnloadAllWallets()
{
    {
        LOCK(cs_WalletManager);
        vector<string> vstrNames;
        vector<boost::shared_ptr<CWallet> > vpWallets;
        BOOST_FOREACH(const wallet_map::value_type& item, wallets)
        {
            vstrNames.push_back(item.first);
            vpWallets.push_back(item.second);
        }
        
        for (unsigned int i = 0; i < vstrNames.size(); i++)
        {
            printf("Unloading wallet %s\n", vstrNames[i].c_str());
            {
                LOCK(vpWallets[i]->cs_wallet);
                UnregisterWallet(vpWallets[i].get());
                wallets.erase(vstrNames[i]);
            }
        }
    }
}

boost::shared_ptr<CWallet> CWalletManager::GetWallet(const string& strName)
{
    {
        LOCK(cs_WalletManager);
        if (!wallets.count(strName))
            throw CWalletManagerException(CWalletManagerException::WALLET_NOT_LOADED,
                                          "CWalletManager::GetWallet() - Wallet not loaded.");
        return wallets[strName];
    }
}

const boost::regex CWalletManager::WALLET_NAME_REGEX("[a-zA-Z0-9_]*");
const boost::regex CWalletManager::WALLET_FILE_REGEX("wallet-([a-zA-Z0-9_]+)\\.dat");

bool CWalletManager::IsValidName(const string& strName)
{
    return boost::regex_match(strName, CWalletManager::WALLET_NAME_REGEX);
}

vector<string> CWalletManager::GetWalletsAtPath(const boost::filesystem::path& pathWallets)
{
    vector<string> vstrFiles = GetFilesAtPath(pathWallets, file_option_flags::REGULAR_FILES);
    vector<string> vstrNames;
    boost::cmatch match;
    BOOST_FOREACH(const string& strFile, vstrFiles)
    {
        if (boost::regex_match(strFile.c_str(), match, CWalletManager::WALLET_FILE_REGEX))
            vstrNames.push_back(string(match[1].first, match[1].second));
    }
    return vstrNames;
}