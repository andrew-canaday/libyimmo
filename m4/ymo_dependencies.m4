# ===========================================================================
#       https://github.com/andrew-canaday/libyimmo/m4/ymo_options.m4
# ===========================================================================
#
# SYNOPSIS
#
#   YMO_CHECK_DEPENDENCIES()
#
# DESCRIPTION
#
#   Libyimmo dependency checks for:
#   - Required programs
#   - Library dependencies
#
# LICENSE
#
#   Copyright (c) 2015 Andrew T. Canaday <http://github.com/andrew-canaday>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.
#
#==============================================================================

#serial 1

# -------------------------------------------------------------------
# YMO_CHECK_DEPENDENCIES()
# -------------------------------------------------------------------
AC_DEFUN([YMO_CHECK_DEPENDENCIES],[
    ##-----------------------------
    ##         Programs:
    ##-----------------------------
    YMO_BOX([Checking for required programs])
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
    YMO_BOX([Checking for required libraries])
    AX_PTHREAD
    AX_LIB_SOCKET_NSL
    YMO_LIB_EV
    AX_CHECK_UUID_UTIL_LINUX([],
    [AC_MSG_ERROR([util-linux libuuid required to build libyimmo])])
    PKG_CHECK_MODULES([BSAT], [libbsat])
    PKG_CHECK_MODULES([PYTHON], [python-3.11-embed], [], [
    		   PKG_CHECK_MODULES([PYTHON], [python-3.10-embed], [YMO_NOTICE([Found python 3.10])], [
    				      PKG_CHECK_MODULES([PYTHON], [python3-embed], [YMO_NOTICE([Found python 3])], [
                              PKG_CHECK_MODULES([PYTHON], [python3], [YMO_NOTICE([Found python 3])])
                              ])
                          ])
    		   ])


    AX_CHECK_OPENSSL([],[AC_MSG_ERROR([openssl required to build libyimmo])])
    PKG_CHECK_MODULES([YAML], [libyaml], [], [
        AC_CHECK_HEADERS([yaml.h])
        AC_CHECK_LIB([yaml],[yaml_get_version_string])

        # Bail if either of yaml.h or -lyaml is not present:
        AS_IF([test "x$ac_cv_header_yaml_h" != "xyes" \
            -o "x$ac_cv_lib_yaml_yaml_get_version_string" != "xyes"],[
            YMO_ERROR([libyaml is required for build!])
        ])
    ])
])

