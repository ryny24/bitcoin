#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([keystore.h])
AM_INIT_AUTOMAKE([gnu])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lboost_unit_test_framework':
AC_CHECK_LIB([boost_unit_test_framework], [main])
# FIXME: Replace `main' with a function in `-lkyotocabinet':
AC_CHECK_LIB([kyotocabinet], [main])
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [main])

AX_BOOST_BASE([1.42.1])
AX_BOOST_SYSTEM([1.42.1])
AX_BOOST_FILESYSTEM([1.42.1])
AX_BOOST_PROGRAM_OPTIONS([1.42.1)]
AX_BOOST_THREAD([1.42.1])

#AX_BERKELEY_DB_CXX([4.8.0], , exit 1)

AX_CHECK_OPENSSL()

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([fdatasync ftruncate gethostname getpagesize gettimeofday iswprint localtime_r memset mkdir munmap pow rmdir select setlocale socket sqrt strchr strdup strerror strrchr strspn strstr strtol strtoull])

AC_CONFIG_FILES([Makefile
                 leveldb/Makefile])
AC_OUTPUT
