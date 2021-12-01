/*=============================================================================
 * test/ymo_basic: Test basic functions of libyimmo.
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
#include "ymo_tap.h"
#include "yimmo.h"
#include "ymo_http_response.h"

/*-------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------*/
#define BUFF_SIZE 2048
static ymo_status_t status;
static ymo_http_response_t* response = NULL;


/*-------------------------------------------------------------*
 * Tests:
 *-------------------------------------------------------------*/
static int create_response(void)
{
    response = ymo_http_response_create(NULL);
    ymo_assert(response != NULL);
    YMO_TAP_PASS("TODO: this isn't really a test!");
}

static int add_headers()
{
    status = ymo_http_response_set_header(
            response, "Server", "ymo_http_response");
    ymo_assert(status == YMO_OKAY);
    status = ymo_http_response_set_header(
            response, "Content-Type", "application/json; charset=UTF-8");
    ymo_assert(status == YMO_OKAY);
    status = ymo_http_response_set_header(
            response, "Content-Length", "0");
    ymo_assert(status == YMO_OKAY);
    YMO_TAP_PASS("Same here...");
}

/*-------------------------------------------------------------*
 * Main:
 *-------------------------------------------------------------*/
YMO_TAP_RUN(YMO_TAP_NO_INIT(),
        YMO_TAP_TEST_FN(create_response),
        YMO_TAP_TEST_FN(add_headers),
        YMO_TAP_TEST_END()
        )

