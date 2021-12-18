#!/usr/bin/env bash
#===============================================================================
#
# NOTICE: THIS AUXILIARY FILE IS LICENSED USING THE MIT LICENSE.
#
# Copyright (c) 2021 Andrew T. Canaday
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


SPHINX_MODE="true"
DEBUG_MODE="false"

dbg() {
    if [ "x${DEBUG_MODE}" == "xtrue" ]; then
        printf "DEBUG: %s\n" "$@" >&2
    fi
}

handle_rst() {
    local comment_file source_file
    comment_file="$1" ; shift
    source_file="$1" ; shift

    cat "${comment_file}"

    if [ -n "${source_file}" -a -r "${source_file}" ]; then
        printf " %s\n\n" '**Declaration:**'
        printf " %s\n\n" '.. code-block:: c'
        cat "${source_file}" \
            | sed 's/^\(.\)/    \1/g'
    fi
}

handle_md() {
    local comment_file source_file
    comment_file="$1" ; shift
    source_file="$1" ; shift

    cat "${comment_file}"

    if [ -n "${source_file}" -a -r "${source_file}" ]; then
        printf "%s\n" '```C'
        cat "${source_file}"
        printf "%s\n\n" '```'
    fi
}

handle_file() {
    local comment_file source_file
    comment_file="$1" ; shift
    source_file="$1" ; shift

    echo ''
    if [ "x${C2SPHINX_P_EXT}" == "xrst" ]; then
        if [ "x${SPHINX_MODE}" == "xtrue" ]; then
            cat "${comment_file}"

            if [ -n "${source_file}" -a -r "${source_file}" ]; then
                cat "${source_file}"
            fi
            echo ''
            return
        else
            handle_rst "${comment_file}" "${source_file}"
        fi
    else
        handle_md "${comment_file}" "${source_file}"
    fi
    echo ''
}

get_output_path() {
    local d_out p_in p_out f_in f_out f_ext
    d_out="$( cd ${1} ; echo ${PWD})" ; shift
    p_in="${1}" ; shift
    f_ext="${1}" ; shift

    f_in="${p_in##*/}"
    f_out="$( sed 's/\.h$/_h.'${C2SPHINX_P_EXT}'/g' <<< "${f_in}" \
        | sed 's/\.c$/_c.'${C2SPHINX_P_EXT}'/g' )"
    p_out="${d_out}/${f_out}"

    echo "${p_out}"
}

main() {
    local path_out
    if [ "x${C2SPHINX_P_DOCDIR}" != "x" ]; then
        path_out="$( get_output_path \
            "${C2SPHINX_P_DOCDIR}" \
            "${C2SPHINX_SOURCE}" \
            "${C2SPHINX_P_EXT}"
        )"
    fi

    dbg "Output file is ${path_out}"

    # If no args: clear the file:
    if [ $# -eq 0 ]; then
        if [ -n "${path_out}" ]; then
            dbg "Clearing: ${path_out}"
            > "${path_out}"
            return 0;
        fi

        return 1;
    fi

    if [ -n "${path_out}" ]; then
        handle_file "$@" >> "${path_out}"
    else
        handle_file "$@"
    fi
}

main "$@"

# EOF

