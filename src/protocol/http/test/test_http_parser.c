/*=============================================================================
 * test/test_http_parser: basic HTTP parser functionality tests
 *
 * Copyright (c) 2014 Andrew Canaday
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ymo_config.h"
#include "ymo_attrs.h"

#include "yimmo.h"
#include "core/ymo_tap.h"
#include "core/ymo_proto.h"
#include "core/ymo_test_proto.h"

#include "ymo_http_test.h"

#include "ymo_http.h"
#include "ymo_proto_http.h"



/*-------------------------------------------------------------*
 * Tests:
 *-------------------------------------------------------------*/
static int expect_100_continue(void)
{
    const char* r_data =
        "POST /body HTTP/1.1\r\n"
        "Host: 127.0.0.1:8081\r\n"
        "Accept: */*\r\n"
        "Expect: 100-continue\r\n"
        "Content-Length: 4\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "\r\n"
        "data";

    const char* expected =
        "HTTP/1.1 100 Continue\r\n"
        "Content-Length: 0\r\n"
        "\r\n"
        "HTTP/1.1 200 OK\r\n"
        "content-type: text/plain\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "OK";

    make_request(r_data);

    /* Confirm: */
    ymo_assert_str_eq(r_info.response_data, expected);
    YMO_TAP_PASS(__func__);
}


static int eagain_on_incomplete_request(void)
{
    const char* r_data = "GET /test-1-1 HTTP/1.0";
    ssize_t r_val = make_request(r_data);

    /* Confirm: */
    ymo_assert(r_val == (ssize_t)strlen(r_data)); /* completely parsed */
    ymo_assert(r_info.called == 0);               /* callback not invoked */
    YMO_TAP_PASS(__func__);
}


/*============
 * 200:
 *------------*/
static int request_basic_1_0(void)
{
    const char* r_data = "GET /test-1-0 HTTP/1.0\r\n\r\n";
    ssize_t r_val = make_request(r_data);

    /* Confirm: */
    ymo_assert(r_val == (ssize_t)strlen(r_data)); /* completely parsed */
    ymo_assert(r_info.called == 1);               /* callback invoked */
    ymo_assert_str_eq(r_info.uri, "/test-1-0");   /* uri correct */
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_200);
    YMO_TAP_PASS(__func__);
}


static int request_basic_1_1(void)
{
    const char* r_data = "GET /test-1-1 HTTP/1.1\r\n\r\n";
    ssize_t r_val = make_request(r_data);

    /* Confirm: */
    ymo_assert(r_val == (ssize_t)strlen(r_data)); /* completely parsed */
    ymo_assert(r_info.called == 1);               /* callback invoked */
    ymo_assert_str_eq(r_info.uri, "/test-1-1");
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_200);
    YMO_TAP_PASS(__func__);
}


/*============
 * 4xx:
 *------------*/
static int test_400_on_bad_method(void)
{
    const char* r_data =
        "GE+ /bad-method HTTP/1.1\r\n"
        "Host: 127.0.0.1:8081\r\n"
        "\r\n";
    make_request(r_data);
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_400);
    YMO_TAP_PASS(__func__);
}


static int test_400_on_bad_version(void)
{
    const char* r_data =
        "GET /bad-version HTTP/1.3\r\n"
        "Host: 127.0.0.1:8081\r\n"
        "\r\n";
    make_request(r_data);
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_400);
    YMO_TAP_PASS(__func__);
}


static int test_400_on_bad_uri(void)
{
    const char* r_data =
        "GET /bad URI HTTP/1.1\r\n"
        "Host: 127.0.0.1:8081\r\n"
        "\r\n";
    make_request(r_data);
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_400);
    YMO_TAP_PASS(__func__);
}


static int test_400_on_bad_header_field(void)
{
    /* Let's try to test every "bad" char possible: */
    const char* r_fmt =
        "GET /bad-header HTTP/1.1\r\n"
        "My-%c-header: value\r\n"
        "\r\n";

    char r_data[512];

    /* TODO: why does it fail for c >= 0x7f!? */
    for( uint8_t c=0; c < 0xff; c++)
    {
        /* TODO: (full disclosure: not catching these, at the moment) */
        if( c == '\0' || c == ':' ) {
            continue;
        }

        /* We skip this one because sprintf will just delete the prior char: */
        if( c == 0x7f ) {
            continue;
        }

        reset_r_info(); /* HACK: reset the test request on each iteration. */
        int is_valid =
            (c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c == '-');
        sprintf(r_data, r_fmt, c);
#if 0
        ymo_log_warning(
                "Testing %s char: '%c' (0x%02x)",
                is_valid ? "VALID" : "INVALID",
                c, (int)(c & 0xff));
        ymo_log_warning(">>%s<<", r_data);
#endif

        make_request(r_data);

        if( is_valid ) {
            ymo_assert_str_eq(r_info.response_data, TEST_HTTP_200);
        } else {
            ymo_assert_str_eq(r_info.response_data, TEST_HTTP_400);
        }
    }
    YMO_TAP_PASS(__func__);
}

#if 0
static int test_400_on_header_field_trailing_space(void)
{
    const char* r_data =
        "GET /trailing-field-space HTTP/1.1\r\n"
        "Name-Value : but-line-invalid\r\n"
        "\r\n";
    make_request(r_data);
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_400);
    YMO_TAP_PASS(__func__);
}

static int test_400_on_missing_header_value(void)
{
    const char* r_data =
        "GET /missing-value HTTP/1.1\r\n"
        "Host\r\n"
        "\r\n";
    make_request(r_data);
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_400);
    YMO_TAP_PASS(__func__);
}


static int test_413_on_large_payload(void)
{
    const char* r_data =
        "POST /large-payload HTTP/1.1\r\n"
        "Host: 127.0.0.1:8081\r\n"
        "Accept: */*\r\n"
        "Content-Length: 330368\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "\r\n";
    make_request(r_data);
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_413);
    YMO_TAP_PASS(__func__);
}


#endif


/*============
 * 5xx:
 *------------*/
static int test_501_on_bad_upgrade(void)
{
    const char* r_data =
        "GET /bad-upgrade HTTP/1.1\r\n"
        "Host: 127.0.0.1:8081\r\n"
        "Upgrade: not-a-protocol\r\n"
        "\r\n";
    make_request(r_data);
    ymo_assert_str_eq(r_info.response_data, TEST_HTTP_501);
    YMO_TAP_PASS(__func__);
}


/*-------------------------------------------------------------*
 * Main:
 *-------------------------------------------------------------*/

static int setup_suite(void)
{
    ymo_log_init();
    /* Instantiate an HTTP protocol instance to test against: */
    ymo_proto_t* http_proto = get_proto_http(http_ok_cb);
    test_server = test_server_create(http_proto);
    ymo_assert(test_server != NULL);

    init_r_info();
    return 0;
}


static int setup_test(void)
{
    /* Reset the request info between each test: */
    reset_r_info();
    ymo_assert(r_info.called == 0);
    return 0;
}


static int cleanup(void)
{
    ymo_proto_http_cleanup(test_server->proto_data, test_server->server);
    ymo_server_free(test_server->server);
    YMO_FREE(test_server);
    return 0;
}


YMO_TAP_RUN(&setup_suite, &setup_test, &cleanup,
        YMO_TAP_TEST_FN(request_basic_1_0),
        YMO_TAP_TEST_FN(request_basic_1_1),
        YMO_TAP_TEST_FN(expect_100_continue),
        YMO_TAP_TEST_FN(eagain_on_incomplete_request),
        YMO_TAP_TEST_FN(test_400_on_bad_method),
        YMO_TAP_TEST_FN(test_400_on_bad_version),
        YMO_TAP_TEST_FN(test_400_on_bad_uri),
        YMO_TAP_TEST_FN(test_400_on_bad_header_field),
        /* YMO_TAP_TEST_FN(test_400_on_header_field_trailing_space), */
        /* YMO_TAP_TEST_FN(test_400_on_missing_header_value), */
        YMO_TAP_TEST_FN(test_501_on_bad_upgrade),
        YMO_TAP_TEST_END()
        )

