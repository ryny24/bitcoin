ACLOCAL_AMFLAGS = -I m4
bin_PROGRAMS=bitcoind

bitcoind_SOURCES=addrman.cpp alert.cpp bitcoinrpc.cpp bloom.cpp checkpoints.cpp crypter.cpp db.cpp hash.cpp init.cpp irc.cpp key.cpp keystore.cpp leveldb.cpp main.cpp netbase.cpp net.cpp noui.cpp protocol.cpp rpcblockchain.cpp rpcdump.cpp rpcmining.cpp rpcnet.cpp rpcrawtransaction.cpp rpcwallet.cpp script.cpp sync.cpp txdb.cpp util.cpp version.cpp wallet.cpp walletdb.cpp

bitcoind_CPPFLAGS=-I$(top_srcdir)/leveldb/include -I$(top_srcdir)/leveldb/helpers

bitcoind_LDADD = \
	@BOOST_SYSTEM_LIB@ \
	@BOOST_FILESYSTEM_LIB@ \
	@BOOST_PROGRAM_OPTIONS_LIB@ \
	@BOOST_THREAD_LIB@ \
	@OPENSSL_LDFLAGS@ \
	@OPENSSL_LIBS@

AM_CXXFLAGS = $(BOOST_CPPFLAGS)
