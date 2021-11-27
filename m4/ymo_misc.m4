# ===========================================================================
#       https://github.com/andrew-canaday/libyimmo/m4/ymo_options.m4
# ===========================================================================
#
# SYNOPSIS
#
#   YMO_BOX([MSG])
#
# DESCRIPTION
#
#   Miscellaneous M4 macros.
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
# YMO_TERM_CHECK()
# -------------------------------------------------------------------
AC_DEFUN([YMO_TERM_CHECK],[
        AS_IF([test -t AS_MESSAGE_FD],[
            ymo_configure_is_term=1

            AS_IF([test "x$YMO_CONFIGURE_COLOR" == "xno"],[
                ymo_configure_has_color=0
            ],[
                ymo_configure_has_color=1

            ])
        ],[
            ymo_configure_is_term=0
            ymo_configure_has_color=0
            AC_MSG_NOTICE([Message output (AS_MESSAGE_FD) is not a terminal])
        ])

    YMO_NOTICE([Output (AS_MESSAGE_FD) is terminal: $ymo_configure_is_term])
    YMO_NOTICE([Output (AS_MESSAGE_FD) has color: $ymo_configure_has_color])
])


# -------------------------------------------------------------------
# YMO_DEFINE_AND_SUB([variable,value,description])
# -------------------------------------------------------------------
AC_DEFUN([YMO_DEFINE_AND_SUB],[
    AC_SUBST([$1],[$2])
    AC_DEFINE_UNQUOTED([$1],[$2],[$3])
])


# -------------------------------------------------------------------
# YMO_DEFINE_STR_AND_SUB([variable,value,description])
# -------------------------------------------------------------------
AC_DEFUN([YMO_DEFINE_STR_AND_SUB],[
    AC_SUBST([$1],[$2])
    AC_DEFINE_UNQUOTED([$1],[$2],[$3])
])


# -------------------------------------------------------------------
# YMO_BOX([MSG])
# -------------------------------------------------------------------
AC_DEFUN([YMO_BOX],[
        AS_IF([test "x$ymo_configure_has_color" == "x1"],[
            printf "\\n\\n\\033@<:@00;03;37;m" >&AS_MESSAGE_FD
            AS_BOX([$1])
            printf "\\033@<:@0;m" >&AS_MESSAGE_FD
        ],[
            printf "\\n\\n" >&AS_MESSAGE_FD
            AS_BOX([$1])
        ])
])


# -------------------------------------------------------------------
# YMO_NOTICE([MSG])
# -------------------------------------------------------------------
AC_DEFUN([YMO_NOTICE],[
        AS_IF([test "x$ymo_configure_has_color" == "x1"],[
            ymo_color_l="\\033@<:@01;04;35;m"
            ymo_color_r="\\033@<:@00;m"
        ])
        printf "${ymo_color_l}NOTICE: %s${ymo_color_r}\\n" "$1" >&AS_MESSAGE_FD
])


# -------------------------------------------------------------------
# YMO_WARN([MSG])
# -------------------------------------------------------------------
AC_DEFUN([YMO_WARN],[
        AS_IF([test "x$ymo_configure_has_color" == "x1"],[
            ymo_color_l="\\033@<:@00;01;04;31;m"
            ymo_color_r="\\033@<:@00;m"
        ])
        printf "${ymo_color_l}WARN: %s${ymo_color_r}\\n" "$1" >&AS_MESSAGE_FD
])


# -------------------------------------------------------------------
# YMO_ERROR([MSG])
# -------------------------------------------------------------------
AC_DEFUN([YMO_ERROR],[
        AS_IF([test "x$ymo_configure_has_color" == "x1"],[
                AC_MSG_ERROR([\\033@<:@0;04;31;m$1\\033@<:@00;m])
            ],[
                AC_MSG_ERROR([$1])
        ])
])


# EOF

