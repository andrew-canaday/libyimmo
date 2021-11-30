/*=============================================================================
 * test/http_test: utilities for mod/http tests
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

#ifndef YMO_HTTP_TEST_H
#define YMO_HTTP_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ymo_tap.h"
#include "yimmo.h"
#include "ymo_net.h"
#include "ymo_proto.h"

#include "ymo_test_proto.h"

#include "ymo_http.h"
#include "ymo_proto_http.h"
#include "ymo_http_response.h"

#define MAX_REQUEST_SIZE  1024
#define MAX_RESPONSE_SIZE 1024

/*-------------------------------------------------------------*
 * We use this bit to verify callback data:
 *-------------------------------------------------------------*/

#define RESPONSE_OK \
    "HTTP/1.1 200 OK\r\n" \
    "content-type: text/plain\r\n" \
    "Content-Length: 2\r\n" \
    "\r\n" \
    "OK"

ymo_test_server_t* test_server = NULL;

struct {
    int     called;
    char    uri[128];
    char    response_data[MAX_RESPONSE_SIZE+1];
    ssize_t bytes_sent;
} r_info;

static ymo_status_t http_ok_cb(
        ymo_http_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user)
{
    r_info.called = 1;
    strcpy(r_info.uri, request->uri);
    ymo_http_response_set_header(response, "content-type", "text/plain");
    ymo_http_response_set_status_str(response, "200 OK");
    ymo_bucket_t* content = YMO_BUCKET_FROM_REF("OK", 2);

    ymo_http_response_body_append(response, content);
    ymo_http_response_finish(response);
    return YMO_OKAY;
}


/*-------------------------------------------------------------*
 * Utility:
 *
 * TODO: tidy these and move to reusable header.
 *-------------------------------------------------------------*/

static void reset_r_info(void)
{
    memset(&r_info, 0, sizeof(r_info));
    return;
}

/* Return an HTTP protocol object that has no data and only
 * registers a HTTP complete callback handler:
 */
static ymo_proto_t* get_proto_http(ymo_http_cb_t http_cb)
{
    if( http_cb == NULL ) {
        http_cb = &http_ok_cb;
    }

    return ymo_proto_http_create(
            NULL, http_cb, NULL, NULL, NULL, NULL);
}

static ssize_t make_request(const char* r_data)
{
    /* Create a client socket/connection: */
    ymo_test_conn_t* test_conn = test_conn_create(test_server);
    if( !test_conn ) {
        return -1;
    }

    ymo_http_session_t* session = ymo_proto_http_conn_init(
            test_server->proto_data, test_conn->conn);

    /* We'll test the parser by instantiating a protocol object
     * and invoking it's read callback, as if we were the
     * server:
     */
    char request_buf[MAX_REQUEST_SIZE];
    strncpy(request_buf, r_data, MAX_REQUEST_SIZE);
    ssize_t r_val = ymo_proto_http_read(
            test_server->proto_data, test_conn->conn,
            session, request_buf, strlen(request_buf));


    /* If all went okay, send the response by faking a write-ready event: */
    if( r_val > 0 ) {
        ymo_proto_http_write(
                test_server->proto_data,
                test_conn->conn,
                session, test_conn->fd_send);

        r_info.bytes_sent = read(
                test_conn->fd_read, r_info.response_data,
                sizeof(r_info.response_data));
    }

    /* Free the test protocol and return: */
    ymo_proto_http_conn_cleanup(
            test_server->proto_data, test_conn->conn, session);
    ymo_conn_free(test_conn->conn);
    YMO_FREE(test_conn);
    return r_val;
}

#endif /* YMO_HTTP_TEST_H */

