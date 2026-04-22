dnl Process this file with autoconf to produce a configure script.

AC_INIT([libcYandexDisk], [1.1])
AM_INIT_AUTOMAKE
LT_INIT

AC_PROG_CC
AC_CANONICAL_HOST
AC_CONFIG_SRCDIR

AC_ARG_ENABLE([winnt], 
							[AS_HELP_STRING([--enable-winnt], [Enable
							 Windows NT 4.0 support @<:@no@:>@])],
							 [enable_winnt=yes],
							 [enable_winnt=no])

AM_CONDITIONAL([WINNT], [test "$enable_winnt" = "yes"])

AC_ARG_ENABLE([withtest], 
							[AS_HELP_STRING([--with-test], [Build
							 'test' executable @<:@no@:>@])],
							 [with_test=yes],
							 [with_test=no])

AM_CONDITIONAL([WITH_TEST], [test "$with_test" = "yes"])

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

