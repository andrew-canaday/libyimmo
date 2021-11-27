# ===========================================================================
#       https://github.com/andrew-canaday/libyimmo/m4/ymo_options.m4
# ===========================================================================
#
# SYNOPSIS
#
#   YMO_OPTION([OPTION_NAME],[DEFAULT_VALUE],[DESCRIPTION])
#   YMO_OPTION_DEPRECATED([OPTION_NAME],[DEFAULT_VALUE],[DESCRIPTION])
#
# DESCRIPTION
#
#   Convenience macros used to declare additional configuration items.
#   Given an option name in the form, SOME_OPTION, do the following:
#    - declare SOME_OPTION as a precious variable
#    - #define YMO_SOME_OPTION in the project config header
#
#   The value used in the header definition will be taken from the input value
#   of the SOME_OPTION precious variable, if set. Otherwise, the default value
#   specified in the macro invocation will be used.
#
#   The description parameter will be included as the help string for
#   SOME_OPTION when "configure --help" is invoked, and as the comment
#   accompanying YMO_SOME_OPTION in the config header file.
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
# YMO_OPTION([OPTION_NAME],[DEFAULT_VALUE],[DESCRIPTION])
# -------------------------------------------------------------------
AC_DEFUN([YMO_OPTION],[
        AC_ARG_VAR([YMO_$1],[$3 (default $2)])
        AS_VAR_SET_IF([YMO_$1],[ymo_opt_val=$$1],[ymo_opt_val=$2])

        AS_IF([test "x$ymo_opt_val" == "x$2"],[
            ymo_color="01;32"
        ],[
            ymo_color="01;33"
        ])

        AS_IF([test "x$ymo_configure_has_color" == "x1"],[
            ymo_color_v="\\033@<:@00;01;36;m"
            ymo_color_l="\\033@<:@00;$ymo_color;m"
            ymo_color_r="\\033@<:@00;m"
        ])

        printf "${ymo_color_v}%-26.26s${ymo_color_r} = ${ymo_color_l}%s${ymo_color_r}\\n" \
            "YMO_$1" \
            "$ymo_opt_val" >&AS_MESSAGE_FD
        AC_DEFINE_UNQUOTED([YMO_$1],[$ymo_opt_val],[$3])
])

# -------------------------------------------------------------------
# YMO_OPTION_DEPRECATED([OPTION_NAME],[DEFAULT_VALUE],[DESCRIPTION])
# -------------------------------------------------------------------
# Convenience macro used to declare additional configuration options which
# are explicitly marked as deprecated
AC_DEFUN([YMO_OPTION_DEPRECATED],[
        YMO_OPTION([$1],[$2],[$3])

        AS_IF([test "x$ymo_opt_val" != "x$2" ],[
            YMO_NOTICE([NOTE: The YMO_$1 configure-time option is deprecated])
        ])
])

# EOF

