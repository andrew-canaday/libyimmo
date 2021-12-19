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

. ${thisdir}/../../util/bash/ymo-utils.sh

AB_CONFIG_DIR="${AB_CONFIG_DIR:-"$( cd ${thisdir} ; echo ${PWD} )/config"}"
AB_REPORTS_DIR="${AB_REPORTS_DIR:-"${PWD}/reports"}"
AB_NAME="${AB_NAME:-"yimmo-autobahn-test"}"

mkdir -p "${AB_REPORTS_DIR}"

hr '='
log_info "Autobahn Tests:"
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

hr '-'
log_info 'Results'
${thisdir}/autobahn-json-summary.sh "${AB_REPORTS_DIR}/servers/index.json"

# TODO: remove duration before diffing...
if [ "x${AB_DO_DIFF}" == "xyes" ]; then
    hr '-'
    log_info 'Diff:'
    diff ${thisdir}/results/yimmo-ws-results.json "${AB_REPORTS_DIR}/servers/index.json"
fi
hr '.'

