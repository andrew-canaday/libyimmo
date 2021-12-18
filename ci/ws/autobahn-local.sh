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

thisdir="$( cd ${0%/*} ; echo "${PWD}" )"

AB_CONFIG_DIR="${AB_CONFIG_DIR:-"$( cd ${thisdir} ; echo ${PWD} )/config"}"
AB_REPORTS_DIR="${AB_REPORTS_DIR:-"${PWD}/reports"}"
AB_NAME="${AB_NAME:-"yimmo-autobahn-test"}"

mkdir -p "${AB_REPORTS_DIR}"

# Sorry, again:
if [ "x$1" == "xstop" ]; then
    docker stop "${AB_NAME}"
else
    docker run -t ${AB_DOCKER_OPTS} \
        --rm \
        --name "${AB_NAME}" \
        --entrypoint "${AB_ENTRYPOINT:-"/opt/pypy/bin/wstest"}" \
        --add-host=host.docker.internal:host-gateway \
        -v "${AB_CONFIG_DIR}:/config" \
        -v "${AB_REPORTS_DIR}:/reports" \
        -p 9001:9001 \
        --name autobahn-tests \
        crossbario/autobahn-testsuite \
            -m fuzzingclient \
            -s "${FUZZINGCLIENT_CONFIG:-"/config/fuzzingclient.local.json"}"
fi
