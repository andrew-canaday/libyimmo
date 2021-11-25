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
    AC_ARG_VAR([$1],[$3 (default $2)])
    AS_VAR_SET_IF([$1],[ymo_opt_val=$$1],[ymo_opt_val=$2])
    AC_MSG_NOTICE([YMO_$1=$ymo_opt_val])
    AC_DEFINE_UNQUOTED([YMO_$1],[$ymo_opt_val],[$3])
])

# -------------------------------------------------------------------
# YMO_OPTION_DEPRECATED([OPTION_NAME],[DEFAULT_VALUE],[DESCRIPTION])
# -------------------------------------------------------------------
# Convenience macro used to declare additional configuration options which
# are explicitly marked as deprecated
AC_DEFUN([YMO_OPTION_DEPRECATED],[
    YMO_OPTION([$1],[$2],[$3])
    AC_MSG_NOTICE([NOTE: The $1 configure-time option is deprecated])
    ])

# EOF

