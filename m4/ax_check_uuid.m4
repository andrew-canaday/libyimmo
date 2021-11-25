# ===========================================================================
#    https://github.com/andrew-canaday/libyimmo/m4/ax_check_uuid_api.m4
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CHECK_UUID([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
#   AX_CHECK_UUID_UTIL_LINUX([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
#   AX_CHECK_UUID_OSSP([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
#
# DESCRIPTION
#
#   Look for a uuid library in a variety of standard locations, or in a user-
#   selected location, provided via --with-uuid.
#
#   Presently, there are two main "libuuid" libraries commonly available to C
#   programmers - the util-linux-ng variant and the OSSP variant. For more info
#   see the following:
#    * Linux-util: https://git.kernel.org/cgit/utils/util-linux/util-linux.git/
#    * OSSP UUID: http://www.ossp.org/pkg/lib/uuid/
#
#   This macro attempts to differentiate between the two - preferring the
#   linux-util implementation over OSSP. If a suitable library is found,
#   the following output variables are defined:
#
#     * UUID_CFLAGS - Compiler flags necessary to build with uuid support
#     * UUID_LIBS - Linker flags necessary to link against the uuid library
#
#   ACTION-IF-FOUND or ACTION-IF-NOT-FOUND are called appropriately.
#
#   Additionally, API-specific variants are also provided, in case you only
#   want to target a specific uuid implementation.
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
# TODO
#   1. Leverage pkg-config to find headers/libs when default search patsh are
#      insufficient for discovery via built-ins.
#   2. AC_DEFINE appropriate values...?
#
# ===========================================================================

#serial 2

# -------------------------------------------------------------------
# AX_CHECK_UUID_UTIL_LINUX([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
# -------------------------------------------------------------------
AC_DEFUN([AX_CHECK_UUID_UTIL_LINUX],[
    ax_cv_have_uuid_util_linux=[0]
    AC_SEARCH_LIBS([uuid_generate],[uuid],[
        AC_CHECK_HEADERS([uuid/uuid.h],[
            AC_CHECK_DECLS([[uuid_generate],[uuid_unparse],[uuid_copy]],[
                    ax_cv_have_uuid_util_linux=[1]
                ],[
                    # util-linux specific functions not found:
                    $2
                ],
                [
                    #include <uuid/uuid.h>
                ])
        ],[
            # uuid/uuid.h header not found:
            $2     
        ])
    ],[
        # no library providing uuid_generate found:
        $2
    ])
    AC_DEFINE_UNQUOTED(
        [HAVE_LIBUUID_UTIL_LINUX_API],
        [$ax_cv_have_uuid_util_linux],
        [Set to true if uuid API is util-linux])
])

# ------------------------------------------------------------
# AX_CHECK_UUID_OSSP([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
# ------------------------------------------------------------
AC_DEFUN([AX_CHECK_UUID_OSSP],[
    AC_SEARCH_LIBS([uuid_create],[uuid],[
        AC_CHECK_HEADERS([uuid.h],[
            # As a temporary hack for darwin systems, we suppress the
            # inclusion of <uuid/uuid.h> by setting _POSIX_C_SOURCE:
            old_CFLAGS="${CFLAGS}"
            old_CPPFLAGS="${CPPFLAGS}"
            export CFLAGS="-D_POSIX_C_SOURCE -D_XOPEN_SOURCE  ${old_CFLAGS}"
            export CPPFLAGS="-D_POSIX_C_SOURCE -D_XOPEN_SOURCE ${old_CPPFLAGS}"
            AC_CHECK_DECLS([[uuid_create],[uuid_destroy],[uuid_clone]],[
                    $1
                ],[
                    # OSSP specific functions not found:
                    # TODO: leverage pkg-config before giving up!
                    $2
                ],
                [
                    #ifdef _DARWIN_C_SOURCE
                    #undef _DARWIN_C_SOURCE
                    #endif /* _DARWIN_C_SOURCE */
                ])
            export CFLAGS="${old_CFLAGS}"
            export CPPFLAGS="${old_CPPFLAGS}"
        ],[
            # uuid.h header not found:
            # TODO: leverage pkg-config before giving up!
            $2     
        ])
    ],[
        # no library providing uuid_create found:
        # TODO: leverage pkg-config before giving up!
        $2
    ])
])


# -------------------------------------------------------
# AX_CHECK_UUID([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
# Prefer util-linux style UUID lib oer OSSP. Search both.
# -------------------------------------------------------
AC_DEFUN([AX_CHECK_UUID],[
    AX_CHECK_UUID_UTIL_LINUX([],[AX_CHECK_UUID_OSSP([$1],[$2])])
])

# EOF

