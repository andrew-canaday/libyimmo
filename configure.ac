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

FLAMBORP="asdf"

##-----------------------------
##           Init:
##-----------------------------
YMO_BOX([Init])
# NOTE: This is the PACKAGE version (SemVer), NOT the ABI version (libtool):
AC_INIT(
    [libyimmo], [0.0.4],
    [andrew.canaday@gmail.com],[libyimmo],
    [https://github.com/andrew-canaday/libyimmo])
AC_CONFIG_AUX_DIR([./build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_USE_SYSTEM_EXTENSIONS


# HACK HACK HACK: color output for "checking..."
YMO_TERM_CHECK()
YMO_MSG_RESULT() AC_MSG_RESULT([$1])
m4_pushdef([AC_MSG_RESULT], [{
        AS_IF([test "x$ymo_configure_has_color" == "x1" ],[
            AS_CASE(["x$1"],
                [xyes*], [ymo_color_l="\\033@<:@00;32;m"],
                [xno*], [ymo_color_l="\\033@<:@01;31;m"],
                [ymo_color_l="\\033@<:@01;m"]
                )
	    ymo_color_r="\\033@<:@00;m"
            ])
        printf "${ymo_color_l}" >&AS_MESSAGE_FD
        YMO_MSG_RESULT $1
        printf "${ymo_color_r}" >&AS_MESSAGE_FD
    }dnl
])

##-----------------------------
##         Metadata:
##-----------------------------
YMO_BOX([Metadata])

## Package Info for build files:
AC_SUBST([PACKAGE_LICENSE],["GPLv3"],[Package License])
AC_SUBST([PACKAGE_VENDOR],["Andrew Canaday"])
AC_COPYRIGHT([© copyright 2014 Andrew Canaday])
AC_SUBST([PACKAGE_COPYRIGHT],["© copyright 2014 Andrew Canaday"],[Copyright Info])
AC_SUBST([PACKAGE_DESCRIPTION],["Lightweight socket server framework"])
AC_SUBST([PACKAGE_SUMMARY],["Lightweight socket server framework"])

# NOTE: ABI versioning is C.R.A. (libtool) not M.m.p (semver)!!
# C.R.A. = Current, Revision, Age:
# - Current: the interface version
# - Age: how many versions prior to "current" current is backwards compat with
# - Revision: how many times current has been released, publicly
#
# Semver translation:
# - when it would be a "patch level" release: bump revision only
# - when it would be a "minor level" release: bump current, revision, AND age
# - when it would be a "major level" release: bump current; age = revision = 0
#
# For more info, see:
# - https://www.gnu.org/software/coretool/manual/html_node/Updating-version-info.html
# - https://autotools.io/coretool/version.html
AC_SUBST([YMO_ABI_CURRENT],[0],[Yimmo libtool ABI current])
AC_SUBST([YMO_ABI_AGE],[0],[Yimmo libtool ABI age])
AC_SUBST([YMO_ABI_REVISION],[4],[Yimmo libtool ABI revision])
AC_SUBST([YMO_LIB_VERSION],["$YMO_ABI_CURRENT:$YMO_ABI_REVISION:$YMO_ABI_AGE"])

## Package Version:
## HACK: get the version components (note: we get awk early here)
AC_PROG_AWK
AC_SUBST([YIMMO_VERSION_MAJOR],[`echo $PACKAGE_VERSION | awk -F. '{print $1}'`])
AC_SUBST([YIMMO_VERSION_MINOR],[`echo $PACKAGE_VERSION | awk -F. '{print $2}'`])
AC_SUBST([YIMMO_VERSION_PATCH],[`echo $PACKAGE_VERSION | awk -F. '{print $3}'`])
AC_SUBST([YIMMO_VERSION],[`echo $VERSION | awk -F. '{printf("0x%02x%02x%02x00", $1, $2, $3)}'`])

## This is the top-level directory into which API headers are installed:
AC_SUBST([YIMMO_INCLUDE],[yimmo-$YIMMO_VERSION_MAJOR.$YIMMO_VERSION_MINOR])
AC_SUBST([YIMMO_INCLUDEDIR],[$includedir/$YIMMO_INCLUDE])

## There are a lot of these, so they're moved into the m4 directory:
YMO_CHECK_DEPENDENCIES
YMO_CHECK_FEATURES

##---------------------------------------------
##        Configuration Options:
##---------------------------------------------
YMO_BOX([Checking user configuration settings])


##-----------------------------
##    Override Options:
##-----------------------------
YMO_OPTION([ALLOC_ALLOW_WEAKREF],[1],
	   [Allow ymo_alloc functions to be overridden using weakref])
YMO_OPTION([ALLOC_ALLOW_WEAK],[1],
	   [Allow ymo_alloc functions to be overridden using weak])
YMO_OPTION([ALLOC_ALLOW_ALIAS],[1],
	   [Allow ymo_alloc functions to be overridden using alias])
YMO_OPTION([HDR_HASH_ALLOW_WEAK],[0],
	   [Allow ymo_http_hdr_hash functions to be overridden using weak])

##-----------------------------
##    Server Options:
##-----------------------------
YMO_OPTION([SERVER_IDLE_TIMEOUT],[5],
    [Default client idle disconnect period in seconds])
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
##      I/O Options:
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
YMO_BOX([Checking feature configuration])
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

##--- PY_WEBSOCKETS: ---
## Add flag to explicitly build py-websockets code, by default. If absent, py-websockets
## programs are compiled as an automake "EXTRA" target:
AC_ARG_ENABLE([py-websockets],
    AS_HELP_STRING(
        [--enable-py-websockets],
        [Build py-websockets programs]))

AM_CONDITIONAL([BUILD_PY_WEBSOCKETS],[test "x$enable_py_websockets" = "xyes"])
AS_IF([test "x$enable_py_websockets" = "xyes"],[
		AC_DEFINE([YIMMO_PY_WEBSOCKETS],[1],[Do build optional websockets support into yimmo-wsgi])
	],[
	   	AC_DEFINE([YIMMO_PY_WEBSOCKETS],[0],[Do not build optional websockets support into yimmo-wsgi])
	])

##--- Maintainer Debug: ---
## Check for maintainer-debug:
AC_ARG_ENABLE([maintainer-debug],
    AS_HELP_STRING(
        [--enable-maintainer-debug],
        [VERY verbose logging for maintainer-debug development]))

## If set, extend CFLAGS with ALL KINDS OF VERBOSE INFORMATION.
AS_IF([test "x$enable_maintainer_debug" = "xyes"], [
    YMO_NOTICE([COMPILING WITH MAINTAINER DEBUG CFLAGS])
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
        -DYMO_LOG_LEVEL_MAX=6 -DYMO_LOG_LEVEL_DEFAULT=5"
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
    YMO_NOTICE([COMPILING WITH VERBOSE ASSERTIONS])
    CFLAGS="${CFLAGS} -DYMO_ASSERT_VERBOSE=1"
    ])


##-----------------------------
##          Output:
##-----------------------------
YMO_BOX([Generating output files])
AC_CONFIG_HEADERS([src/core/ymo_config.h])

AC_CONFIG_LINKS([examples/index.html:examples/index.html])

## yimmo general output files:
AC_CONFIG_FILES([
    Makefile
    src/Makefile
    examples/Makefile
    wsgi/Makefile
    benchmarks/Makefile
    src/protocol/Makefile
    ])

## libyimmo core output files:
AC_CONFIG_FILES([
    include/yimmo.h
    include/Makefile
    src/core/Makefile
    src/core/test/Makefile
    src/core/libyimmo-${YIMMO_VERSION_MAJOR}.${YIMMO_VERSION_MINOR}.pc:src/core/libyimmo.pc.in
    ])

## libyimmo_http output files:
AC_CONFIG_FILES([
    src/protocol/http/Makefile
    src/protocol/http/include/Makefile
    src/protocol/http/test/Makefile
    src/protocol/http/libyimmo_http-${YIMMO_VERSION_MAJOR}.${YIMMO_VERSION_MINOR}.pc:src/protocol/http/libyimmo_http.pc.in
    ])

## libyimmo_ws output files:
AC_CONFIG_FILES([
    src/protocol/ws/Makefile
    src/protocol/ws/include/Makefile
    src/protocol/ws/test/Makefile
    src/protocol/ws/libyimmo_ws-${YIMMO_VERSION_MAJOR}.${YIMMO_VERSION_MINOR}.pc:src/protocol/ws/libyimmo_ws.pc.in
    ])

## libyimmo_mqtt output files:
AC_CONFIG_FILES([
    src/protocol/mqtt/Makefile
    src/protocol/mqtt/include/Makefile
    src/protocol/mqtt/test/Makefile
    src/protocol/mqtt/libyimmo_mqtt-${YIMMO_VERSION_MAJOR}.${YIMMO_VERSION_MINOR}.pc:src/protocol/mqtt/libyimmo_mqtt.pc.in
    ])

## libyimmo doc output files:
AM_EXTRA_RECURSIVE_TARGETS([docs manual])
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

# Test stuff:
AC_REQUIRE_AUX_FILE([tap-driver.sh])

AC_OUTPUT
YMO_BOX([libyimmo configuration complete])

## end configure.ac
