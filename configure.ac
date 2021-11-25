# Run autoconf on this file to produce configure.in
##==============================================================================
##  libyimmo
##
##  Copyright (c) 2014 Andrew Canaday
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##
##==============================================================================


##-----------------------------
##           Init:
##-----------------------------
AS_BOX([Init])
AC_INIT(
    [libyimmo], [0.0.0],
    [andrew.canaday@gmail.com],[libyimmo],
    [https://github.com/andrew-canaday/libyimmo])
AC_CONFIG_AUX_DIR([./build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_USE_SYSTEM_EXTENSIONS


##-----------------------------
##         Metadata:
##-----------------------------
AS_BOX([Metadata])
AC_DEFINE([PACKAGE_LICENSE],["GPLv3"],[Package License])
AC_SUBST([PACKAGE_VENDOR],["Andrew Canaday"])
AC_COPYRIGHT([© copyright 2014 Andrew Canaday])
AC_DEFINE([PACKAGE_COPYRIGHT],
    ["© copyright 2014 Andrew Canaday"],
    [Copyright Info])
AC_SUBST([PACKAGE_COPYRIGHT],["© copyright 2014 Andrew Canaday"])
AC_SUBST([PACKAGE_DESCRIPTION],
    ["Lightweight socket server framework"])
AC_SUBST([PACKAGE_SUMMARY],["Lightweight socket server framework"])
AC_SUBST([YMO_LIB_VERSION],[0:0:0])

## HACK: get the version components (note: we get awk early here)
AC_PROG_AWK
AC_SUBST([PACKAGE_MAJOR],[`echo $PACKAGE_VERSION | awk -F. '{print $1}'`])
AC_SUBST([PACKAGE_MINOR],[`echo $PACKAGE_VERSION | awk -F. '{print $2}'`])
AC_SUBST([PACKAGE_PATCH],[`echo $PACKAGE_VERSION | awk -F. '{print $3}'`])

## This is the top-level directory into which API headers are installed:
AC_SUBST([YIMMO_INCLUDE],[yimmo-$PACKAGE_MAJOR.$PACKAGE_MINOR])
AC_SUBST([YIMMO_INCLUDEDIR],[$includedir/$YIMMO_INCLUDE])


##-----------------------------
##         Programs:
##-----------------------------
AS_BOX([Checking for required programs])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_PROG_CC
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT
PKG_PROG_PKG_CONFIG([0.24])
AC_PROG_LN_S
# TODO: make these subject to --enable/--disable-docs
AC_PROG_SED
AC_PATH_PROG([POMD4C],[pomd4c])
AM_CONDITIONAL([HAVE_POMD4C], [test "x$POMD4C" != "x"])
AC_PATH_PROG([C2SPHINX],[c2sphinx])
AM_CONDITIONAL([HAVE_C2SPHINX], [test "x$C2SPHINX" != "x"])
AC_PATH_PROG([PLANTUML],[plantuml])
AM_CONDITIONAL([HAVE_PLANTUML], [test "x$PLANTUML" != "x"])


##-----------------------------
##         Libraries:
##-----------------------------
AS_BOX([Checking for required libraries])
AX_PTHREAD
AX_LIB_SOCKET_NSL
YMO_LIB_EV
AX_CHECK_UUID_UTIL_LINUX([],
    [AC_MSG_ERROR([util-linux libuuid required to build libyimmo])])
PKG_CHECK_MODULES([BSAT], [libbsat])
PKG_CHECK_MODULES([PYTHON], [python-3.9-embed], [AC_MSG_NOTICE([Found python 3.9])], [
		   PKG_CHECK_MODULES([PYTHON], [python-3.8-embed], [AC_MSG_NOTICE([Found python 3.8])], [
				      PKG_CHECK_MODULES([PYTHON], [python3], [AC_MSG_NOTICE([Found python 3])])
				      ])
		   ])


AX_CHECK_OPENSSL([],[AC_MSG_ERROR([openssl required to build libyimmo])])


##-----------------------------
##           Headers:
##-----------------------------
AS_BOX([Checking for required C header files])
AC_HEADER_ASSERT
AC_CHECK_HEADERS([stdarg.h stdbool.h stddef.h])
AC_CHECK_HEADERS([sys/socket.h sys/time.h])
AC_CHECK_HEADERS([netinet/in.h])


##-----------------------------
##           Types:
##-----------------------------
#
# TODO:
# - in_port_t
# - socklen_t
# - uuid_t
##-----------------------------
AS_BOX([Checking for required types])
AC_TYPE_OFF_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINTMAX_T
AC_TYPE_UINTPTR_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_SIZEOF([size_t])


##-----------------------------
##    Arch/Compiler Traits:
##-----------------------------
AS_BOX([Checking compiler features])
AC_C_BIGENDIAN
AC_C_RESTRICT
#AC_C_VOLATILE
AC_C_INLINE
AC_C_STRINGIZE
AC_C_FLEXIBLE_ARRAY_MEMBER
AC_C_VARARRAYS
AC_C_TYPEOF


##-----------------------------
##    Compiler Attributes:
##-----------------------------
AS_BOX([Checking for available compile C attributes])
#AX_GCC_FUNC_ATTRIBUTE([access])
#AX_GCC_FUNC_ATTRIBUTE([alloc_align])
AX_GCC_FUNC_ATTRIBUTE([alias])
AX_GCC_FUNC_ATTRIBUTE([alloc_size])
AX_GCC_FUNC_ATTRIBUTE([always_inline])
AX_GCC_FUNC_ATTRIBUTE([cold])
AX_GCC_FUNC_ATTRIBUTE([const])
AX_GCC_FUNC_ATTRIBUTE([deprecated])
AX_GCC_FUNC_ATTRIBUTE([flatten])
AX_GCC_FUNC_ATTRIBUTE([format])
AX_GCC_FUNC_ATTRIBUTE([gnu_inline])
AX_GCC_FUNC_ATTRIBUTE([hot])
AX_GCC_FUNC_ATTRIBUTE([malloc])
AX_GCC_FUNC_ATTRIBUTE([nonnull])
#AX_GCC_FUNC_ATTRIBUTE([noplt])
#AX_GCC_FUNC_ATTRIBUTE([pure])
#AX_GCC_FUNC_ATTRIBUTE([returns_nonnull])
#AX_GCC_FUNC_ATTRIBUTE([simd])
AX_GCC_FUNC_ATTRIBUTE([unused])
AX_GCC_FUNC_ATTRIBUTE([used])
AX_GCC_FUNC_ATTRIBUTE([warn_unused_result])
AX_GCC_FUNC_ATTRIBUTE([weak])
AX_GCC_FUNC_ATTRIBUTE([weakref])
AX_GCC_FUNC_ATTRIBUTE([fallthrough])


##-----------------------------
##       Function Checks:
##-----------------------------
AS_BOX([Checking for required functions])
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([strcasecmp])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([strtol])
YMO_CHECK_SOCKET_API


##-----------------------------
##    Configuration Options:
##-----------------------------
AS_BOX([Checking user configuration settings])
YMO_OPTION_DEPRECATED([SERVER_IDLE_TIMEOUT],[5],
    [Client idle disconnect period in seconds])
YMO_OPTION_DEPRECATED([SERVER_RECV_BUF_SIZE],[8192],
    [Server receive buffer size for calls to recv])


##-----------------------------
##    HTTP Options:
##-----------------------------
YMO_OPTION_DEPRECATED([HTTP_RECV_BUF_SIZE],[1024],
    [Per-request receive buffer size])
YMO_OPTION_DEPRECATED([HTTP_REQ_WS_SIZE],[1024],
    [Per-request request buffer size])
YMO_OPTION_DEPRECATED([HTTP_SEND_BUF_SIZE],[1024],
    [Per-response send buffer size])


##-----------------------------
##    MQTT Options:
##-----------------------------
YMO_OPTION_DEPRECATED([MQTT_RECV_BUF_SIZE],[4096],
    [MQTT session receive buffer size])


##-----------------------------
##    Advanced Options:
##-----------------------------
## Server settings:
YMO_OPTION([BUCKET_MAX_IOVEC],[32],
    [Maximum sendmsg/writemsg iovec array size])

## Installation options:
m4_ifdef([PKG_INSTALLDIR],[PKG_INSTALLDIR],[
    AC_SUBST([pkgconfigdir],[$libdir/pkgconfig])
])

##-----------------------------
##          Features:
##-----------------------------
AS_BOX([Checking feature configuration])
##--- Examples: ---
## Add flag to explicitly build example code, by default. If absent, example
## programs are compiled as an automake "EXTRA" target:
AC_ARG_ENABLE([examples],
    AS_HELP_STRING(
        [--enable-examples],
        [Build example programs]))

##--- WSGI: ---
## Add flag to explicitly build wsgi code, by default. If absent, wsgi
## programs are compiled as an automake "EXTRA" target:
AC_ARG_ENABLE([wsgi],
    AS_HELP_STRING(
        [--enable-wsgi],
        [Build wsgi programs]))

AM_CONDITIONAL([BUILD_WSGI],[test "x$enable_wsgi" = "xyes"])
AS_IF([test "x$enable_wsgi" = "xyes"],[
		AC_DEFINE([YIMMO_WSGI],[1],[Build yimmo wsgi module])
	],[
	   	AC_DEFINE([YIMMO_WSGI],[0],[Do not build the yimmo wsgi module])
	])

##--- Maintainer Debug: ---
## Check for maintainer-debug:
AC_ARG_ENABLE([maintainer-debug],
    AS_HELP_STRING(
        [--enable-maintainer-debug],
        [VERY verbose logging for maintainer-debug development]))

## If set, extend CFLAGS with ALL KINDS OF VERBOSE INFORMATION.
AS_IF([test "x$enable_maintainer_debug" = "xyes"], [
    AC_MSG_NOTICE([COMPILING WITH MAINTAINER DEBUG CFLAGS])
    CFLAGS="${CFLAGS} \
        -Og -g \
        -std=c11 -pedantic \
        -fstrict-aliasing \
        -Wall \
        -Wextra \
        -Winline \
        -Wundef\
        -Wno-unused-parameter \
        -Wno-error=nonnull \
        -Wno-error=inline \
        -Wno-error=unused-parameter \
        -Wno-error=strict-aliasing \
        -Wno-error=missing-field-initializers \
        -Wno-error=implicit-fallthrough \
        -DYMO_LOG_LEVEL_MAX=6 -DYMO_LOG_LEVEL_DEFAULT=5 \
        -DYMO_ASSERT_VERBOSE=1"
    ])
AM_CONDITIONAL([MAINTAINER_DEBUG],[test "x$enable_maintainer_debug" = "xyes"])

##--- Example Build ---
## Building of example code happens if *either* maintainer-debug or
## build-examples is specified:
AM_CONDITIONAL([BUILD_EXAMPLES],
    [test "x$enable_examples" = "xyes" -o "x$enable_maintainer_debug" = "xyes"])

##--- Benchmarks: ---
## Add flag to explicitly build benchmark code, by default. If absent, benchmark
## programs are compiled as an automake "EXTRA" target:
AC_ARG_ENABLE([benchmarks],
    AS_HELP_STRING(
        [--enable-benchmarks],
        [Build benchmark programs]))

## Building of benchmark code happens if *either* maintainer-debug or
## build-benchmarks is specified:
## TODO: looks like "no," dude. Fix it up!
AM_CONDITIONAL([BUILD_BENCHMARKS],[test "x$enable_benchmarks" = "xyes"])

##--- Testing/Assertions: ---
## Check for verbose-assert:
AC_ARG_ENABLE([verbose-assert],
    AS_HELP_STRING(
        [--enable-verbose-assert],
        [Turn verbose assertions on for tests]))

## If set, extend CFLAGS with verbose assertions
AS_IF([test "x$enable_verbose_assert" = "xyes"], [
    AC_MSG_NOTICE([COMPILING WITH VERBOSE ASSERTIONS])
    CFLAGS="${CFLAGS} -DYMO_ASSERT_VERBOSE=1"
    ])


##-----------------------------
##          Output:
##-----------------------------
AS_BOX([Generating output files])
AC_CONFIG_HEADERS([ymo_config.h])

AC_CONFIG_LINKS([example/index.html:example/index.html])

## yimmo package output files:
AC_CONFIG_FILES([
    Makefile
    libyimmo-$PACKAGE_MAJOR.$PACKAGE_MINOR.pc:libyimmo.pc.in
    ])

## libyimmo core output files:
AC_CONFIG_FILES([
    lib/Makefile
    include/Makefile
    lib/test/Makefile
    example/Makefile
    wsgi/Makefile
    benchmarks/Makefile
    mod/Makefile
    ])

## libyimmo_http output files:
AC_CONFIG_FILES([
    mod/http/Makefile
    mod/http/include/Makefile
    mod/http/test/Makefile
    ])

## libyimmo_ws output files:
AC_CONFIG_FILES([
    mod/ws/Makefile
    mod/ws/include/Makefile
    mod/ws/test/Makefile
    ])

## libyimmo_mqtt output files:
AC_CONFIG_FILES([
    mod/mqtt/Makefile
    mod/mqtt/include/Makefile
    mod/mqtt/test/Makefile
    ])

## libyimmo doc output files:
AM_EXTRA_RECURSIVE_TARGETS([docs])
AC_CONFIG_FILES([
    doc/Makefile
    doc/conf.py
    doc/core/Makefile
    doc/http/Makefile
    doc/ws/Makefile
    doc/mqtt/Makefile
    doc/wsgi/Makefile
    doc/examples/Makefile
    doc/diagrams/Makefile
    ])
AC_REQUIRE_AUX_FILE([tap-driver.sh])

AC_OUTPUT
AS_BOX([libyimmo configuration complete])

## end configure.ac
