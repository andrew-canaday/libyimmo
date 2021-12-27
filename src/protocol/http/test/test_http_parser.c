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
static int request_basic_1_0(void)
{
    const char* r_data = "GET /test-1-0 HTTP/1.0\r\n\r\n";
    ssize_t r_val = make_request(r_data);

    /* Confirm: */
    ymo_assert(r_val == (ssize_t)strlen(r_data)); /* completely parsed */
    ymo_assert(r_info.called == 1);               /* callback invoked */
    ymo_assert_str_eq(r_info.uri, "/test-1-0");   /* uri correct */
    ymo_assert_str_eq(r_info.response_data, RESPONSE_OK);
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
    ymo_assert_str_eq(r_info.response_data, RESPONSE_OK);
    YMO_TAP_PASS(__func__);
}


static int expect_100_continue(void)
{
    const char* r_data =
        "POST /body HTTP/1.1\r\n" \
        "Host: 127.0.0.1:8081\r\n" \
        "User-Agent: curl/7.64.1\r\n" \
        "Accept: */*\r\n" \
        "Expect: 100-continue\r\n" \
        "Content-Length: 4\r\n" \
        "Content-Type: application/x-www-form-urlencoded\r\n" \
        "\r\n" \
        "data";

    const char* expected =
        "HTTP/1.1 100 Continue\r\n" \
        "Content-Length: 0\r\n\r\n";

    make_request(r_data);

    /* Confirm: */
    ymo_assert_str_eq(r_info.response_data, expected);
    YMO_TAP_PASS(__func__);
}


static int einval_on_bad_version(void)
{
    const char* r_data = "GET /test-1-1 HTTP/53.2\r\n\r\n";
    ssize_t r_val = make_request(r_data);

    /* Confirm: */
    ymo_assert(r_val == -1);         /* parse error returned */
    ymo_assert(errno == EINVAL);     /* payload was invalid */
    ymo_assert(r_info.called == 0);  /* callback not invoked */
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


/*-------------------------------------------------------------*
 * Main:
 *-------------------------------------------------------------*/

static int setup_suite(void)
{
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
        YMO_TAP_TEST_FN(einval_on_bad_version),
        YMO_TAP_TEST_FN(eagain_on_incomplete_request),
        YMO_TAP_TEST_END()
        )

