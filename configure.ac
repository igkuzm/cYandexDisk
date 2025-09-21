dnl Process this file with autoconf to produce a configure script.

AC_INIT([libcYandexDisk], [1.1])
AM_INIT_AUTOMAKE

LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_PROG_CXX

AC_CANONICAL_HOST

AC_CONFIG_SRCDIR

case "${host_os}" in
    cygwin*|mingw*)
        build_windows=yes;;
    darwin*)
        build_mac=yes;;
    *)
#check curl headers
		AC_CHECK_HEADER([curl/curl.h],[],[AC_MSG_ERROR([Please install libcurl])],[])
		build_linux=yes;;
esac

case "${host_cpu}" in
    i?86*)
        build_i386=yes;;
    x86_64*)
        build_x86_64=yes;;
    arm64*|aarch64*)
        build_arm64=yes;;
    armv7*)
        build_arm=yes;;
    *)
		;;
esac

AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([WIN32], [test "$build_windows" = "yes" -a "$build_i386" == "yes"])
AM_CONDITIONAL([WIN64], [test "$build_windows" = "yes" -a "$build_x86_64" == "yes"])


AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT

