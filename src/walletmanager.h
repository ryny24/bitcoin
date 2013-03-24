// Copyright (c) 2012-2013 Eric Lombrozo, The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_WALLETMANAGER_H
#define BITCOIN_WALLETMANAGER_H

#include "wallet.h"

#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

class CWalletManagerException : public std::runtime_error
{
public:
    enum ErrorType
    {
        WALLET_NOT_LOADED,
        UNKNOWN_ERROR
    };
    ErrorType type;
    
    CWalletManagerException(ErrorType _type, const char* message) : std::runtime_error(message), type(_type)  { }
};

/** A CWalletManager handles loading, unloading, allocation, deallocation, and synchronization of wallet objects.
 */
typedef std::map<std::string, boost::shared_ptr<CWallet> > wallet_map;
class CWalletManager
{
protected:
    static const boost::regex WALLET_NAME_REGEX;
    static const boost::regex WALLET_FILE_REGEX;
    
    mutable CCriticalSection cs_WalletManager;
    wallet_map wallets;
    
public:
    ~CWalletManager() { UnloadAllWallets(); }
    
    bool LoadWallet(const std::string& strName, std::ostringstream& strErrors);
    bool UnloadWallet(const std::string& strName);
    void UnloadAllWallets();
    
    // GetWallet and GetDefaultWallet throw a CWalletManagerException if the wallet is not found.
    boost::shared_ptr<CWallet> GetWallet(const std::string& strName);
    boost::shared_ptr<CWallet> GetDefaultWallet() { return GetWallet(""); }
    
    int GetWalletCount() { return wallets.size(); }
    wallet_map GetWalletMap() { return wallets; }
    bool HaveWallet(const std::string& strName) { return (wallets.count(strName) > 0); }
    
    static bool IsValidName(const std::string& strName);
    static std::vector<std::string> GetWalletsAtPath(const boost::filesystem::path& pathWallets);
};

#endif // BITCOIN_WALLETMANAGER_H