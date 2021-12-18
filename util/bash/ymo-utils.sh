#!/usr/bin/env bash
#===============================================================================
#
# NOTICE: THIS AUXILIARY FILE IS LICENSED USING THE MIT LICENSE.
#
# Copyright (c) 2014 Andrew T. Canaday
# This file is licensed under the MIT license.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
#-------------------------------------------------------------------------------

#===============================================================================
#
# utility: Generic utility functions
#
#-------------------------------------------------------------------------------

thisdir="${0%/*}"
abs_thisdir="$( cd "${thisdir}" ; echo "${PWD}" )"
thisname="${0##*/}"

#--------------------------------------
#       Shell Characteristics:
#--------------------------------------

shell_is_interactive() {
    case "$-" in
        *i*)
            return 0;
            ;;
        *)
            return 1;
            ;;
    esac
}

stdout_is_terminal() {
    if [ -t 1 ]; then
        return 0;
    else
        return 1;
    fi
}

stderr_is_terminal() {
    if [ -t 2 ]; then
        return 0;
    else
        return 1;
    fi
}


#--------------------------------------
#              Verbose:
#--------------------------------------

# No-op function
dry_run() {
    printf '\e[00;02;34mSKIP: %s\e[00m\n' "$@" >&2
}

# Print input args to STDERR and exit 1
err_bail() {
    printf '\e[00;31mFATAL: %s\e[00m\n' "$@" >&2
    exit 1
}

# Log to stderr
log_debug() {
    printf '\e[00;36mDEBUG: \e[00;36m%s\e[00m\n' "$@" >&2
}

log_info() {
    printf '\e[00;32mINFO: \e[00m%s\e[00m\n' "$@" >&2
}

log_warn() {
    printf '\e[00;33mWARN: %s\e[00m\n' "$@" >&2
}

log_error() {
    printf '\e[00;31mERROR: %s\e[00m\n' "$@" >&2
}

log_src_debug() {
    printf '\e[00;02m%s \e[00;36mDEBUG: %s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

log_src_info() {
    printf '\e[00;02m%s \e[00;32mINFO: \e[00m%s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

log_src_warn() {
    printf '\e[00;02m%s \e[00;33mWARN: %s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

log_src_error() {
    printf '\e[00;02m%s \e[00;01;31mERROR: %s\e[00m\n' \
        "${BASH_SOURCE[1]}:${FUNCNAME[1]}:${BASH_LINENO[0]}" "$@" >&2
}

# Print the value of a variable with name label.
# $1     - Variable name as a string
print_var() {
    local var_name
    var_name="$1"
    printf "\e[00;34m%s\e[00m=\"\e[00;33m%s\e[00m\"\n" "${var_name}" "${!var_name}"
}

# Run a command, logging the path and arguments
run_cmd() {
    local cmd
    cmd="$1" ; shift
    echo -e "\e[00;02mEXEC: \e[00;32m${cmd} $@\e[00m" >&2
    ${DRY} ${cmd} "$@"
}

hr() {
    local hdr_char
    hdr_char="$1" ; shift
    printf "\n%*s\n" "${COLUMNS:-$( tput cols )}" '' \
        | tr ' ' "${hdr_char[0]}"
}



