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

#include "ymo_tap.h"
#include "ymo_log.h"
#include "ymo_http.h"
#include "ymo_http_hdr_table.h"

/* Common (ish) HTTP headers from all over the place to illustrate
 * unique id's for each with the current hash algorithm.
 *
 * TODO: a follow-up list with collisions as a reminder to put some
 *       strcmp's in ymo_http_hdr_hash!
 */
static const char* LOTS_OF_HEADERS[] = {
    "A-IM",
    "Accept",
    "Accept-CH",
    "Accept-Charset",
    "Accept-Datetime",
    "Accept-Encoding",
    "Accept-Language",
    "Accept-Patch",
    "Accept-Ranges",
    "Access-Control-Allow-Credentials",
    "Access-Control-Allow-Headers",
    "Access-Control-Allow-Methods",
    "Access-Control-Allow-Origin",
    "Access-Control-Expose-Headers",
    "Access-Control-Max-Age",
    "Access-Control-Request-Headers",
    "Age",
    "Allow",
    "Alt-Svc",
    "Alternate-Protocol",
    "Authorization",
    "CF-IPCountry",
    "CF-RAY",
    "CF-Visitor",
    "CF-Worker",
    "Cache-Control",
    "Calling-Version",
    "Cdn-Loop",
    "Clacks",
    "Client-Date",
    "Client-Ip",
    "Client-Peer",
    "Client-Response-Num",
    "Clientip",
    "Connection",
    "Content-Disposition",
    "Content-Encoding",
    "Content-Language",
    "Content-Length",
    "Content-Location",
    "Content-MD5",
    "Content-Range",
    "Content-Security-Policy",
    "Content-Security-Policy-Report-Only",
    "Content-Type",
    "Cookie",
    "DNT",
    "Date",
    "Delta-Base",
    "Device-Stock-Ua",
    "Distilqa",
    "Downlink",
    "Dpc-Locality",
    "Dpc-Version",
    "Dpr",
    "ETag",
    "Expect",
    "Expect-CT",
    "Expires",
    "Field",
    "Forwarded",
    "From",
    "Front-End-Https",
    "Ged-Cache",
    "Giga-Transport",
    "HTTP",
    "HTTP2-Settings",
    "Hadesurlheader",
    "Host",
    "Http-X-Tls-Gls",
    "IM",
    "If-Match",
    "If-Modified-Since",
    "If-None-Match",
    "If-Range",
    "If-Unmodified-Since",
    "Keep-Alive",
    "Last-Modified",
    "Link",
    "Location",
    "Max-Forwards",
    "NEL",
    "Ocp-Apim-Subscription-Key",
    "Origin",
    "P3P",
    "Permissions-Policy",
    "Pragma",
    "Prefer",
    "Preference-Applied",
    "Proxy-Authenticate",
    "Proxy-Authorization",
    "Proxy-Connection",
    "Public-Key-Pins",
    "Range",
    "Referer",
    "Refresh",
    "Report-To",
    "Restrict-Access-Context",
    "Restrict-Access-To-Tenants",
    "Retry-After",
    "Save-Data",
    "Sec-Ch-Ua",
    "Sec-Ch-Ua-Arch",
    "Sec-Ch-Ua-Full-Version",
    "Sec-Ch-Ua-Mobile",
    "Sec-Ch-Ua-Model",
    "Sec-Ch-Ua-Platform",
    "Sec-Ch-Ua-Platform-Version",
    "Sec-Datadog",
    "Sec-Fetch-Dest",
    "Sec-Fetch-Mode",
    "Sec-Fetch-Site",
    "Sec-Fetch-User",
    "Sec-Gpc",
    "Sec-Websocket-Extensions",
    "Sec-Websocket-Key",
    "Sec-Websocket-Protocol",
    "Sec-Websocket-Version",
    "Serial-Number",
    "Server",
    "Set-Cookie",
    "Status",
    "Strict-Transport-Security",
    "Surrogate-Capability",
    "TE",
    "Timing-Allow-Origin",
    "Tk",
    "Trailer",
    "Transfer-Encoding",
    "True-Client-IP",
    "UA-Color",
    "UA-Cpu",
    "UA-Disp",
    "UA-OS",
    "UA-Pixels",
    "Upgrade",
    "Upgrade-Insecure-Requests",
    "User-Agent",
    "Userip",
    "Vary",
    "Via",
    "Viewport-Width",
    "WWW-Authenticate",
    "Warning",
    "X-ATT-DeviceId",
    "X-Akamai-A2-Disable",
    "X-Akamai-Debug",
    "X-Akamai-Edgescape",
    "X-Akamai-Pragma-Headers",
    "X-Akamai-Ro-Piez",
    "X-Akamai-Rua-Debug",
    "X-Amazon-Customer-Id",
    "X-Amazon-Remote-User",
    "X-Amzn-Base-Uri",
    "X-Amzn-User-Roles",
    "X-Amzn-Vendor-Id",
    "X-App",
    "X-Aspnet-Version",
    "X-Atlas-Id",
    "X-Auth-Token",
    "X-Auth-Username",
    "X-Authenticated-Group",
    "X-Authenticated-User",
    "X-B3-Flags",
    "X-B3-Sample",
    "X-B3-Sampled",
    "X-Bluecoat-Authorization",
    "X-Bluecoat-Via",
    "X-Clacks-Overhead",
    "X-Client-Data",
    "X-Client-Ip",
    "X-Content-Duration",
    "X-Content-Security-Policy",
    "X-Content-Type-Options",
    "X-Cookiesok",
    "X-Correlation-ID",
    "X-Csrf-Token",
    "X-Cydia-Cf",
    "X-Device-User-Agent",
    "X-Disney-Internal-Site",
    "X-Distil-Debug",
    "X-Enrichmentstatus",
    "X-Fbs-Client-Ip",
    "X-Forwarded-Email",
    "X-Forwarded-For",
    "X-Forwarded-Groups",
    "X-Forwarded-Host",
    "X-Forwarded-Proto",
    "X-Forwarded-User",
    "X-Frame-Options",
    "X-Googapps-Allowed-Domains",
    "X-Http-Method-Override",
    "X-I",
    "X-IWSaaS-Via",
    "X-Icm",
    "X-If-Via",
    "X-Im-Piez",
    "X-Imforwards",
    "X-Int",
    "X-Iorg-Fbs",
    "X-Iorg-Fbs-Mode",
    "X-Iorg-Fbs-Uip",
    "X-Iws-Via",
    "X-Lastline-Status",
    "X-Lppalgoliasearchv2",
    "X-Lppnewmyaccount",
    "X-Lppnewnavigationmobile",
    "X-Lppnewpageheader",
    "X-Lppnewpickuppoint",
    "X-Lppquerysuggestionenabled",
    "X-Lppsidebar",
    "X-Machine",
    "X-Mobileiron-User-Dn",
    "X-Mwg-Via",
    "X-NewRelic-App-Data",
    "X-NewRelic-Id",
    "X-NewRelic-Transaction",
    "X-Nitro-Pass",
    "X-Nitro-User",
    "X-Nokia-Msisdn",
    "X-Nokia-MusicShop-Bearer",
    "X-Nokia-MusicShop-Version",
    "X-Nokia-imsi",
    "X-Opera-Info",
    "X-Operamini-Features",
    "X-Operamini-Phone",
    "X-Operamini-Phone-Ua",
    "X-Operamini-Route",
    "X-Pdfcrowd",
    "X-Permitted-Cross-Domain-Policies",
    "X-Pingback",
    "X-Powered-By",
    "X-Proxyuser-Ip",
    "X-Puffin-UA",
    "X-Real-Ip",
    "X-Redirect-By",
    "X-Remote-Addr",
    "X-Remote-Ip",
    "X-Request-Id",
    "X-Requested-With",
    "X-Robots-Tag",
    "X-Server-Ip",
    "X-Target",
    "X-Target-Proxy",
    "X-UA-Compatible",
    "X-UCBrowser-Device-UA",
    "X-UIDH",
    "X-Ucbrowser-Device",
    "X-Ucbrowser-Phone",
    "X-Ucbrowser-Phone-Ua",
    "X-Ucbrowser-Ua",
    "X-Up-Subno",
    "X-Url-Category",
    "X-Wap-Profile",
    "X-Wap-Proxy-Cookie",
    "X-WebKit-CSP",
    "X-Websensehost",
    "X-Websenseproxychannel",
    "X-Websenseproxysslconnection",
    "X-Wix-Fastly-Debug",
    "X-Wp-Engine",
    "X-Wvs-Id",
    "X-XSS-Protection",
    "X-Yodel-Auth-Metadata",
    "Z-Forwarded-For",
};


static int test_add_and_get(void)
{
    ymo_http_hdr_table_t* table = ymo_http_hdr_table_create();
    ymo_http_hdr_table_add(table, "Hello", "World");
    ymo_assert_str_eq("World", ymo_http_hdr_table_get(table, "Hello"));
    ymo_http_hdr_table_free(table);
    YMO_TAP_PASS(__func__);
}

static int test_insert_and_get(void)
{
    ymo_http_hdr_table_t* table = ymo_http_hdr_table_create();

    ymo_http_hdr_table_insert(table, "Hello", "World");
    ymo_assert_str_eq("World", ymo_http_hdr_table_get(table, "Hello"));

    ymo_http_hdr_table_insert(table, "Hello", "Dave");
    ymo_assert_str_eq("Dave", ymo_http_hdr_table_get(table, "Hello"));

    ymo_http_hdr_table_free(table);
    YMO_TAP_PASS(__func__);
}

static int test_multiple(void)
{
    ymo_http_hdr_table_t* table = ymo_http_hdr_table_create();

    ymo_http_hdr_table_add(table, "Key", "Value1");
    ymo_assert_str_eq("Value1", ymo_http_hdr_table_get(table, "Key"));

    ymo_http_hdr_table_add(table, "Key", "Value2");
    ymo_assert_str_eq("Value1,Value2", ymo_http_hdr_table_get(table, "Key"));

    ymo_http_hdr_table_free(table);
    YMO_TAP_PASS(__func__);
}

static int test_clear(void)
{
    ymo_http_hdr_table_t* table = ymo_http_hdr_table_create();
    ymo_http_hdr_table_add(table, "Hello", "World");
    ymo_assert_str_eq("World", ymo_http_hdr_table_get(table, "Hello"));
    ymo_http_hdr_table_clear(table);
    ymo_assert_str_eq(NULL, ymo_http_hdr_table_get(table, "Hello"));
    ymo_http_hdr_table_free(table);
    YMO_TAP_PASS(__func__);
}

static int test_iteration(void)
{
    ymo_http_hdr_table_t* table = ymo_http_hdr_table_create();
    ymo_http_hdr_table_add(table, "Key1", "Value1");
    ymo_http_hdr_table_add(table, "Key2", "Value2");
    ymo_http_hdr_table_add(table, "Key3", "Value3");

    const char* hdr_name;
    const char* hdr_value;
    size_t name_len;
    ymo_http_hdr_ptr_t iter = ymo_http_hdr_table_next(
            table, NULL, &hdr_name, &name_len, &hdr_value);

    size_t no_hdr = 0;
    int saw1 = 0;
    int saw2 = 0;
    int saw3 = 0;
    while( iter )
    {
        no_hdr++;

        ymo_log_debug("Got header \"%.*s=%s\"",
                (int)name_len, hdr_name, hdr_value);

        /* Inelegant, but it'll do: */
        if( !strncmp(hdr_name, "Key1", name_len) ) {
            ymo_assert_str_eq(hdr_value, "Value1");
            saw1 = 1;
        }

        if( !strncmp(hdr_name, "Key2", name_len) ) {
            ymo_assert_str_eq(hdr_value, "Value2");
            saw2 = 1;
        }

        if( !strncmp(hdr_name, "Key3", name_len) ) {
            ymo_assert_str_eq(hdr_value, "Value3");
            saw3 = 1;
        }

        iter = ymo_http_hdr_table_next(
                table, iter, &hdr_name, &name_len, &hdr_value);
    }

    ymo_assert(saw1);
    ymo_assert(saw2);
    ymo_assert(saw3);
    ymo_assert(no_hdr == 3);
    YMO_TAP_PASS(__func__);
}

/* Mostly for illustration / reminder purposes: */
static int test_collisions()
{
    size_t no_collisions = 0;
    size_t no_hdrs = sizeof(LOTS_OF_HEADERS)/sizeof(const char*);
    ymo_http_hdr_id_t* hdr_ids = calloc(no_hdrs, sizeof(ymo_http_hdr_id_t));

    /* Stick the ID's in a table: */
    for( size_t i=0; i<no_hdrs; i++ )
    {
        hdr_ids[i] = ymo_http_hdr_hash_283_5(LOTS_OF_HEADERS[i], NULL);

        /* Confirm that this ID hasn't already appeared or else
         * just print out info that it has: */
        for( size_t j=0; j<i; j++ )
        {
            if( hdr_ids[j] == hdr_ids[i]
                    && strcasecmp(LOTS_OF_HEADERS[i], LOTS_OF_HEADERS[j])) {
                ymo_log_debug("Collision: \"%s\" => %i <= \"%s\"\n",
                        LOTS_OF_HEADERS[i], (int)hdr_ids[i], LOTS_OF_HEADERS[j]
                        );
                no_collisions++;
            }
        }
    }

    ymo_log_debug("Total collisions: %zu\n", no_collisions);
    ymo_assert(no_collisions == 0);
    YMO_TAP_PASS(__func__);
}

YMO_TAP_RUN(NULL,
        YMO_TAP_TEST_FN(test_add_and_get),
        YMO_TAP_TEST_FN(test_insert_and_get),
        YMO_TAP_TEST_FN(test_multiple),
        YMO_TAP_TEST_FN(test_clear),
        YMO_TAP_TEST_FN(test_iteration),
        YMO_TAP_TEST_FN(test_collisions),
        YMO_TAP_TEST_END()
        )



