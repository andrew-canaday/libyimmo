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


. ${0%/*}/bash/ymo-utils.sh

WSVC_HOST="${1}" ; shift
WSVC_PORT="${1}" ; shift
WSVC_PATH="${1}" ; shift
WSVC_NO_ITER="${1}" ; shift
WSVC_DELAY="${1}" ; shift

WSVC_URL="http://${WSVC_HOST}:${WSVC_PORT}/${WSVC_PATH}"

log_info "Waiting on service to come up on \"${WSVC_URL}\""
for i in $( seq ${WSVC_NO_ITER} ); do
    log_info "Check $i / ${WSVC_NO_ITER}"
    curl -s -f \
        http://${WSVC_HOST}:${WSVC_PORT}/${WSVC_PATH} \
            -o "Got response:\n%{json}\n\n" \
        >&2 \
        && exit 0

    log_info "Service not up. Waiting ${WSVC_DELAY}s"
    sleep "${WSVC_DELAY}"
done

log_info "Service failed to come up after ${WSVC_NO_ITER} tries. Failing."
exit 1

