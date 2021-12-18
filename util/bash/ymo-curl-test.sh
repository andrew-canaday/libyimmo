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

#===============================================================================
#
# Hacky curl/bash HTTP 1.0 test utilities
#
#-------------------------------------------------------------------------------
CURL="${CURL:-"$( type -p curl )"}"
JQ="${JQ:-"$( type -p jq )"}"

#===============================================================================
#
# CURL TESTS:
#
#-------------------------------------------------------------------------------

REQ_CURL_MAJOR="7"
REQ_CURL_MINOR="70"

DEFAULT_CURL_OPTS=(
    "-s"
    "-S"
    "--connect-timeout" "1.0"
    "--fail-early"
    "--max-time" "2.0"
)

CURL_OPTS="${CURL_OPTS:-"${DEFAULT_CURL_OPTS[@]}"}"

curl_or_die() {
    if [ -z "${CURL}" ]; then
        err_bail "CURL not found (try setting the CURL env var)"
    fi

    local v_str v_info
    v_str="$( "${CURL}" -V | awk 'NR==1{print $2}')"
    v_info=($(echo ${v_str//./ }))

    if [ -z "${v_str}" -o "${#v_info[@]}" -lt 2 ]; then
        err_bail "Unable to determine curl version: \"${v_str}\""
    fi

    if [ ${v_info[1]} -lt ${REQ_CURL_MINOR} -a ${v_info[0]} -lt ${REQ_CURL_MAJOR} ]; then
        err_bail "curl version is ${v_str}. Require >= ${REQ_CURL_MAJOR}.${REQ_CURL_MINOR}"
    fi

    log_info "CURL version: ${v_str}"
}


jq_or_die() {
    if [ -z "${JQ}" ]; then
        err_bail "JQ not found (try setting the JQ env var)"
    fi

    log_info "JQ version: $( jq -V )"
}

ctest() {
    local c_result_status r_val

    r_val=0
    c_result_status="${0##*/}-response.status"
    > "${c_result_status}"

    run_cmd "${CURL}" \
        ${CURL_OPTS} \
        -o /dev/null \
        -w '%{http_code}' \
        "$@" >"${c_result_status}"

    if [[ $? != 0 ]]; then
        return 1
    fi

    if [ -n "${EXPECT_STATUS}" ]; then
        ctest_status "${c_result_status}" "${EXPECT_STATUS}" || r_val=1
    fi

    rm -f "${c_result_status}"
    return ${r_val}
}

ctest_status() {
    local c_status

    if [[ -z "$1" ]] || [[ -z "$2" ]]; then
        log_error "Expected two args. Got $#."
        return 1
    fi

    c_status="$( cat "$1" | tr -d ' \t\n\r' )"

    if [[ ${c_status} != ${2} ]]; then
        log_error "Expected status: \"$2\". Got: \"${c_status}\""
        return 1
    fi

    echo "${c_status}"
    return 0
}

ctest_json() {
    local c_result_json r_val

    r_val=0
    c_result_json="${0##*/}-response.json"
    > "${c_result_json}"

    run_cmd "${CURL}" \
        ${CURL_OPTS} \
        -o /dev/null \
        -w '%{json}' \
        "$@" >"${c_result_json}"

    if [[ $? != 0 ]]; then
        return 1
    fi

    if [ -n "${EXPECT_STATUS}" ]; then
        ctest_status_json "${c_result_json}" "${EXPECT_STATUS}" || r_val=1
    fi

    rm -f "${c_result_json}"

    return ${r_val}
}

ctest_status_json() {
    local c_status

    if [[ -z "$1" ]] || [[ -z "$2" ]]; then
        log_error "Expected two args. Got $#."
        return 1
    fi

    c_status="$( cat "$1" | "${JQ}" -r '.http_code')"

    if [[ ${c_status} != ${2} ]]; then
        log_error "Expected status: \"$2\". Got: \"${c_status}\""
        return 1
    fi

    echo "${c_status}"
    return 0
}

