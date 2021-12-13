#!/usr/bin/env bash

. ${0%/*}/bash/util.sh

YMO_HTTP_TEST_URL_ROOT="${1:-"http://127.0.0.1:8081"}"
TESTS_BUILDDIR="${PWD}"
TESTS_SRCDIR="$( cd ${0%/*} ; echo ${PWD} )"

. ${TESTS_SRCDIR}/bash/util.sh

CTEST_LOG=${TESTS_BUILDDIR}/${0##*/}.trs
CTEST_ERR_LOG=${TESTS_BUILDDIR}/${0##*/}.err

log_info "Results file: ${CTEST_LOG}"
log_info "Error log: ${CTEST_ERR_LOG}"

curl_or_die
jq_or_die

test_basic_status() {
    EXPECT_STATUS="200" \
        ctest -0 "${YMO_HTTP_TEST_URL_ROOT}/status"
}

test_conn_close() {
    EXPECT_STATUS="200" \
        ctest -0 "${YMO_HTTP_TEST_URL_ROOT}/status" \
        -0 "${YMO_HTTP_TEST_URL_ROOT}/status"
}

test_keep_alive() {
    EXPECT_STATUS="200" \
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

    local c_test test_num

    echo "1..${#c_tests[@]}"

    test_num=1
    for c_test in ${c_tests[@]}; do
        ${c_test} >> ${CTEST_LOG} 2>>${CTEST_ERR_LOG}

        if [ $? -eq 0 ]; then
            echo "ok ${test_num} - ${c_test}"
        else
            echo "not ok ${test_num} - ${c_test}"
            printf "    %s\n" "$( tail -n 1 ${CTEST_ERR_LOG} )"
        fi

        test_num=$(( test_num + 1 ))
    done
}

run_tests
