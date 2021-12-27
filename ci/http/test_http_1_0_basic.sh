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


. ${0%/*}/../../util/bash/ymo-utils.sh
. ${0%/*}/../../util/bash/ymo-curl-test.sh

YMO_HTTP_TEST_URL_ROOT="${1:-"http://127.0.0.1:8081"}"
TESTS_BUILDDIR="${PWD}"
TESTS_SRCDIR="$( cd ${0%/*} ; echo ${PWD} )"

CTEST_LOG=${TESTS_BUILDDIR}/${0##*/}.trs
CTEST_ERR_LOG=${TESTS_BUILDDIR}/${0##*/}.err

> "${CTEST_LOG}"
> "${CTEST_ERR_LOG}"

log_info "Results file: ${CTEST_LOG}"
log_info "Error log: ${CTEST_ERR_LOG}"

curl_or_die
jq_or_die

test_basic_status() {
    EXPECT_STATUS="200" \
        ctest -0 "${YMO_HTTP_TEST_URL_ROOT}/status"
}

test_conn_close() {
    EXPECT_STATUS="200OK200" \
        ctest -0 "${YMO_HTTP_TEST_URL_ROOT}/status" \
        -0 "${YMO_HTTP_TEST_URL_ROOT}/status"
}

test_keep_alive() {
    EXPECT_STATUS="200OK200" \
        ctest -0 -H 'Connection: keep-alive' "${YMO_HTTP_TEST_URL_ROOT}/status" \
        -0 -H 'Connection: close' "${YMO_HTTP_TEST_URL_ROOT}/status"
}

test_not_found() {
    EXPECT_STATUS="404" \
        ctest -0 "${YMO_HTTP_TEST_URL_ROOT}/not-a-real-URL"
}

test_post() {
    EXPECT_STATUS="200" \
        ctest -0 \
        --data-binary "${0}" \
        "${YMO_HTTP_TEST_URL_ROOT}/echo/body"
}

test_expect_100_continue() {
    EXPECT_STATUS="200" \
        ctest -0 \
        -H 'Expect: 100-continue' \
        --data-binary "${0}" \
        "${YMO_HTTP_TEST_URL_ROOT}/echo/body"
}

test_te_chunked() {
    EXPECT_STATUS="200" \
        ctest -0 \
        -H 'Transfer-Encoding: chunked' \
        --data-binary "${0}" \
        "${YMO_HTTP_TEST_URL_ROOT}/echo/body"
}


run_tests() {
    local -a c_tests

    c_tests=($( grep -o '^test_\w\w*' "${0}" ))

    local c_test test_num result

    result=0
    echo "1..${#c_tests[@]}"

    test_num=1
    for c_test in ${c_tests[@]}; do
        ${c_test} >> ${CTEST_LOG} 2>>${CTEST_ERR_LOG}

        if [ $? -eq 0 ]; then
            echo "ok ${test_num} - ${c_test}"
        else
            echo "not ok ${test_num} - ${c_test}"
            printf "    %s\n" "$( tail -n 1 ${CTEST_ERR_LOG} )"
            result=1
        fi

        test_num=$(( test_num + 1 ))
    done

    return $result
}

if ! run_tests ; then
    log_error "${0##*/} failed:"
    printf "\033[00;31;m" >&2
    cat "${CTEST_ERR_LOG}" >&2
    printf "\033[00;m" >&2
    exit 1
fi

exit 0
