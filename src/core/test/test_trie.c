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

#include "yimmo.h"
#include "core/ymo_tap.h"
#include "core/ymo_assert.h"
#include "core/ymo_trie.h"

/*-------------------------------------------------------------*
 * Globals:
 *-------------------------------------------------------------*/
static const char* req_headers[] = {
    "Accept",
    "Accept-Charset",
    "Accept-Datetime",
    "Accept-Encoding",
    "Accept-Language",
    "Authorization",
    "Cache-Control",
    "Connection",
    "Content-Length",
    "Content-MD5",
    "Content-Type",
    "Cookie",
    "DNT",
    "Date",
    "Expect",
    "From",
    "Front-End-Https",
    "Host",
    "If-Match",
    "If-Modified-Since",
    "If-None-Match",
    "If-Range",
    "If-Unmodified-Since",
    "Max-Forwards",
    "Origin",
    "Pragma",
    "Proxy-Authorization",
    "Proxy-Connection",
    "Range",
    "Referer",
    "TE",
    "Upgrade",
    "User-Agent",
    "Via",
    "Warning",
    "X-ATT-DeviceId",
    "X-Forwarded-For",
    "X-Forwarded-Proto",
    "X-Http-Method-Override",
    "X-Requested-With",
    "X-Wap-Profile",
    NULL,
};

static int i;
static int hdr_id;
static const char* hdr = NULL;
static ymo_status_t r_val;
static ymo_trie_t* trie = NULL;
static ymo_oitrie_t* oitrie = NULL;


/*-------------------------------------------------------------*
 * Tests:
 *-------------------------------------------------------------*/
static int add_headers(void)
{
    i = 0;
    while( (hdr = req_headers[i++]) != NULL )
    {
        r_val = ymo_trie_add_string(trie, hdr);
        ymo_assert(r_val == YMO_OKAY);
    }
    YMO_TAP_PASS(__func__);
}


static int create_oitrie(void)
{
    oitrie = ymo_oitrie_create(trie);
    ymo_assert(oitrie != NULL);
    YMO_TAP_PASS(__func__);
}


static int check_headers(void)
{
    i = 0;
    while( (hdr = req_headers[i++]) != NULL )
    {
        r_val = ymo_oitrie_get_id(&hdr_id, oitrie, hdr);
        ymo_assert(r_val == YMO_OKAY);
    }
    YMO_TAP_PASS(__func__);
}


static int check_missing(void)
{
    /* Try a name that doesn't exist: */
    hdr = "Not-A-Real-Header";
    r_val = ymo_oitrie_get_id(&hdr_id, oitrie, hdr);
    ymo_assert(r_val == EINVAL);
    YMO_TAP_PASS(__func__);
}


static int check_partial1(void)
{
    /* Try a name that's close: */
    hdr = "Accept-Charse";
    r_val = ymo_oitrie_get_id(&hdr_id, oitrie, hdr);
    ymo_assert(r_val == EINVAL);
    YMO_TAP_PASS(__func__);
}


static int check_partial2(void)
{
    /* Try a name that's close: */
    hdr = "Accept-Charset-";
    r_val = ymo_oitrie_get_id(&hdr_id, oitrie, hdr);
    ymo_assert(r_val == EINVAL);
    YMO_TAP_PASS(__func__);
}


/*-------------------------------------------------------------*
 * Main:
 *-------------------------------------------------------------*/
static int trie_setup(void)
{
    static int setup_done = 0;

    if( !setup_done ) {
        setup_done = 1;
        hdr = req_headers[0];
        trie = ymo_trie_create();
        ymo_assert(trie != NULL);
    }
    return 0;
}


YMO_TAP_RUN(trie_setup, NULL, NULL,
        YMO_TAP_TEST_FN(add_headers),
        YMO_TAP_TEST_FN(create_oitrie),
        YMO_TAP_TEST_FN(check_headers),
        YMO_TAP_TEST_FN(check_missing),
        YMO_TAP_TEST_FN(check_partial1),
        YMO_TAP_TEST_FN(check_partial2),
        YMO_TAP_TEST_END()
        )

