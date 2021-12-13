# ===========================================================================
#       https://github.com/andrew-canaday/libyimmo/m4/ymo_options.m4
# ===========================================================================
#
# SYNOPSIS
#
#   YMO_CHECK_FEATURES()
#
# DESCRIPTION
#
#   Libyimmo configure check stanzas for:
#   - Headers
#   - Types
#   - Architectural traits
#   - Compile Traits
#   - Functions
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
# YMO_CHECK_FEATURES()
# -------------------------------------------------------------------
AC_DEFUN([YMO_CHECK_FEATURES],[
    ##-----------------------------
    ##           Headers:
    ##-----------------------------
    YMO_BOX([Checking for required C header files])
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
    YMO_BOX([Checking for required types])
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
    YMO_BOX([Checking compiler features])
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
    YMO_BOX([Checking for available compile C attributes])
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
    AX_GCC_FUNC_ATTRIBUTE([pure])
    #AX_GCC_FUNC_ATTRIBUTE([returns_nonnull])
    AX_GCC_FUNC_ATTRIBUTE([unused])
    AX_GCC_FUNC_ATTRIBUTE([used])
    AX_GCC_FUNC_ATTRIBUTE([warn_unused_result])
    AX_GCC_FUNC_ATTRIBUTE([weak])
    AX_GCC_FUNC_ATTRIBUTE([weakref])
    AX_GCC_FUNC_ATTRIBUTE([fallthrough])


    ##-----------------------------
    ##       Function Checks:
    ##-----------------------------
    YMO_BOX([Checking for required functions])
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
])

