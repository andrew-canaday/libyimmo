#!/usr/bin/env bash
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

export TERM="${TERM:-"xterm"}"

. ${0%/*}/../../util/bash/ymo-utils.sh
json_path="$1" ; shift

hr '='
log_info "Autobahn WebSocket Server Test Suite Results"

if [ "x${FULL_JSON}" != "xfalse" ]; then
    cat "${json_path}" | jq --color-output '.' >&2
fi


hr '-'
log_info "Autobahn WebSocket Server Test Suite Summary"
log_info "PASSED: $( cat "${json_path}" \
        | jq -r '[ .ymo_test_server | to_entries | .[] | select(.value.behavior == "OK") ] | length'
    )"

log_info "FAILED: $( cat "${json_path}" \
    | jq -r '[ .ymo_test_server | to_entries | .[] | select(.value.behavior == "FAILED") ] | length'
    )"

log_info "NON-STRICT: $( cat "${json_path}" \
        | jq -r '[ .ymo_test_server | to_entries | .[] | select(.value.behavior == "NON-STRICT") ] | length'
    )"


hr '.'
