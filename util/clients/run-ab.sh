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


function run_cmd() {
    local cmd
    cmd="$1" ; shift
    echo -e "\e[00;02mEXEC: \e[00;32m${cmd} $@\e[00m" >&2
    ${DRY} ${cmd} "$@"
}

if [ -z "${YMO_NO_KEEPALIVE}" ]; then
    YMO_KEEPALIVE="-k"
fi

run_cmd ab \
    ${YMO_KEEPALIVE} \
    ${YMO_AB_VERBOSITY} \
    $@ \
    -r \
    -n ${YMO_NO_CLIENTS:-"100000"} \
    -c ${YMO_NO_CONCURRENT:-"32"} \
    "${YMO_TEST_URL:-"http://127.0.0.1:8081/status"}"

# EOF

