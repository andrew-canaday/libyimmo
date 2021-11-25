# ===========================================================================
#        https://github.com/andrew-canaday/libyimmo/m4/ymo_lib_ev.m4
# ===========================================================================
#
# SYNOPSIS
#
#   YMO_LIB_EV()
#
# DESCRIPTION
#
#    Most libev distributions lack a pkg-config file. This macro is used to
#    verify that the required libev headers and shared/static libs are
#    available for compiling and linking.
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

#serial 2

# -------------------------------------------------------------------
# YMO_LIB_EV()
# -------------------------------------------------------------------
AC_DEFUN([YMO_LIB_EV],[
    # libev frequently ships without a pkg-config file, so we check manually:
    AC_CHECK_HEADERS([ev.h])
    AC_CHECK_LIB([ev],[ev_version_major])

    # Bail if either of ev.h or -lev is not present:
    AS_IF([test "x$ac_cv_header_ev_h" != "xyes" \
        -o "x$ac_cv_lib_ev_ev_version_major" != "xyes"],[
        AC_MSG_ERROR([libev is required for build!])
    ])
])

# EOF

