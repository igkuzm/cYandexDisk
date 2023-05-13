dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_CONFIG_SRCDIR([configure.ac])
AM_INIT_AUTOMAKE(libcYandexDisk, 0.1)
AM_MAINTAINER_MODE

LT_INIT
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC

AC_CANONICAL_HOST
case "${host_os}" in
    cygwin*|mingw*)
        build_windows=yes;;
    darwin*)
        build_mac=yes;;
    *)
				build_linux=yes;;
esac
AM_CONDITIONAL([LINUX],   [test "$build_linux"   = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([MACOS],   [test "$build_mac"     = "yes"])

case "${host_cpu}" in
    i386*)
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
AM_CONDITIONAL([I386],    [test "$build_i386"    = "yes"])
AM_CONDITIONAL([X86_64],  [test "$build_x86_64"  = "yes"])
AM_CONDITIONAL([ARM64],   [test "$build_arm64"   = "yes"])
AM_CONDITIONAL([ARM],     [test "$build_arm"     = "yes"])



AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT

