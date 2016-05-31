#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([gMaxLined], [1.0], [shoutrain_goo@aliyun.com])
AC_COPYRIGHT([GPL])
AC_CONFIG_SRCDIR([src/gMaxLinked.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_PROG_RANLIB
# FIXME: Replace `main' with a function in `-lhiredis':
AC_CHECK_LIB([hiredis], [redisConnectWithTimeout])
# FIXME: Replace `main' with a function in `-llog4c':
AC_CHECK_LIB([log4c], [log4c_init])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stddef.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([alarm bzero gettimeofday inet_ntoa memset select socket strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/common/Makefile
                 src/config/Makefile
                 src/database/Makefile
                 src/transaction/Makefile])
AC_OUTPUT
