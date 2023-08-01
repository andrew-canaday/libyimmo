/*=============================================================================
 *
 * Copyright (c) 2014 Andrew Canaday
 *
 * This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
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



#ifndef YMO_HTTP_HDR_TABLE_H
#define YMO_HTTP_HDR_TABLE_H
#include <stdint.h>
#include "yimmo.h"
#include "ymo_http.h"

/** HTTP Header Tables
 * ====================
 *
 * HTTP header hash table.
 */

#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) \
    && (HAVE_FUNC_ATTRIBUTE_WEAK == 1) \
    && defined(YMO_HTTP_HDR_HASH_ALLOW_WEAK) \
    && YMO_HTTP_HDR_HASH_ALLOW_WEAK

/*---------------------------------------------------------------------------*
 * WEAK override:
 *--------------------------------------------------------------------------*/
#  define YMO_HTTP_HDR_HASH_OVERRIDE_METHOD "weak"
#  define YMO_HDR_HASH_FN ymo_http_hdr_hash
#  define YMO_HDR_HASH_CH ymo_http_hdr_hash_ch
#  define YMO_HTTP_HDR_HASH_INIT ymo_http_hdr_hash_init

/** Default hash function used by header table.
 *
 */
YMO_FUNC_PURE ymo_http_hdr_id_t ymo_http_hdr_hash_init(void);
YMO_FUNC_PURE ymo_http_hdr_id_t ymo_http_hdr_hash(const char* str_in, size_t* len);
YMO_FUNC_PURE ymo_http_hdr_id_t ymo_http_hdr_hash_ch(ymo_http_hdr_id_t h, char c);

#else

/*---------------------------------------------------------------------------*
 * PREPROC override:
 *--------------------------------------------------------------------------*/
#  define YMO_HTTP_HDR_HASH_OVERRIDE_METHOD "preproc"
#  ifndef YMO_HDR_HASH_FN
#    define YMO_HDR_HASH_FN ymo_http_hdr_hash_283_5
#  endif /* YMO_HDR_HASH_FN */

#  ifndef YMO_HDR_HASH_CH
#    define YMO_HDR_HASH_CH(h,c) ((h*283) + (c & 0xdf))
#  endif /* YMO_HDR_HASH_CH */

#  ifndef YMO_HTTP_HDR_HASH_INIT
#    define YMO_HTTP_HDR_HASH_INIT() 5
#  endif /* YMO_HTTP_HDR_HASH_INIT */

__attribute__((YMO_FUNC_PURE_P, YMO_FUNC_UNUSED_A))
static inline ymo_http_hdr_id_t ymo_http_hdr_hash_283_5(
        const char* str_in, size_t* len)
{
    const char* hdr_start = str_in;
    char c;
    ymo_http_hdr_id_t h = YMO_HTTP_HDR_HASH_INIT();
    while( (c = *str_in++) ) {
        h = YMO_HDR_HASH_CH(h,c);
    }

    if( len ) {
        *len = (size_t)(str_in - hdr_start)-1;
    }
    return h & YMO_HDR_TABLE_MASK;
}


#endif /* HAVE_FUNC_ATTRIBUTE_WEAK && YMO_HTTP_HDR_HASH_ALLOW_WEAK */



/* Flags */
#define YMO_HDR_FLAG_DEFAULT      0x00
#define YMO_HDR_FLAG_NOPOOL       0x01
#define YMO_HDR_FLAG_MULTI        0x02


/** */
void ymo_http_hdr_table_init(ymo_http_hdr_table_t* table);

/** */
ymo_http_hdr_id_t ymo_http_hdr_table_insert_precompute(
        ymo_http_hdr_table_t* table,
        ymo_http_hdr_id_t h_id,
        const char* hdr,
        size_t hdr_len,
        const char* value);

/** */
ymo_http_hdr_id_t ymo_http_hdr_table_add_precompute(
        ymo_http_hdr_table_t* table,
        ymo_http_hdr_id_t h_id,
        const char* hdr,
        size_t hdr_len,
        const char* value);

/** */
const char* ymo_http_hdr_table_get_id(
        const ymo_http_hdr_table_t* table, ymo_http_hdr_id_t h_id);


/*----------------------------------------------------------------------------
 * IDs for common headers:
 *----------------------------------------------------------------------------*/

/*
 * .. warning::
 *    Don't rely on these.
 *
 * .. todo::
 *    Compute these at build/runtime or something. These are the keys for
 *    the 283/5 algorithm on uint16_t == not gonna work everywhere.
 */

#define YMO_HTTP_HID_A_IM                                   19549  /* "A-IM" */
#define YMO_HTTP_HID_ACCEPT                                   597  /* "Accept" */
#define YMO_HTTP_HID_ACCEPT_CH                               8637  /* "Accept-CH" */
#define YMO_HTTP_HID_ACCEPT_CHARSET                         27356  /* "Accept-Charset" */
#define YMO_HTTP_HID_ACCEPT_DATETIME                         1387  /* "Accept-Datetime" */
#define YMO_HTTP_HID_ACCEPT_ENCODING                        17935  /* "Accept-Encoding" */
#define YMO_HTTP_HID_ACCEPT_LANGUAGE                          220  /* "Accept-Language" */
#define YMO_HTTP_HID_ACCEPT_PATCH                           14124  /* "Accept-Patch" */
#define YMO_HTTP_HID_ACCEPT_RANGES                           7566  /* "Accept-Ranges" */
#define YMO_HTTP_HID_ACCESS_CONTROL_ALLOW_CREDENTIALS       24108  /* "Access-Control-Allow-Credentials" */
#define YMO_HTTP_HID_ACCESS_CONTROL_ALLOW_HEADERS           26806  /* "Access-Control-Allow-Headers" */
#define YMO_HTTP_HID_ACCESS_CONTROL_ALLOW_METHODS           18578  /* "Access-Control-Allow-Methods" */
#define YMO_HTTP_HID_ACCESS_CONTROL_ALLOW_ORIGIN            31630  /* "Access-Control-Allow-Origin" */
#define YMO_HTTP_HID_ACCESS_CONTROL_EXPOSE_HEADERS          12101  /* "Access-Control-Expose-Headers" */
#define YMO_HTTP_HID_ACCESS_CONTROL_MAX_AGE                 17818  /* "Access-Control-Max-Age" */
#define YMO_HTTP_HID_ACCESS_CONTROL_REQUEST_HEADERS         14004  /* "Access-Control-Request-Headers" */
#define YMO_HTTP_HID_ACCESS_CONTROL_REQUEST_METHOD           4999  /* "Access-Control-Request-Method" */
#define YMO_HTTP_HID_AGE                                    30026  /* "Age" */
#define YMO_HTTP_HID_ALLOW                                  14532  /* "Allow" */
#define YMO_HTTP_HID_ALT_SVC                                 4375  /* "Alt-Svc" */
#define YMO_HTTP_HID_ALTERNATE_PROTOCOL                      9616  /* "Alternate-Protocol" */
#define YMO_HTTP_HID_AUTHORIZATION                          10104  /* "Authorization" */
#define YMO_HTTP_HID_CACHE_CONTROL                          23577  /* "Cache-Control" */
#define YMO_HTTP_HID_CLIENT_DATE                             4867  /* "Client-Date" */
#define YMO_HTTP_HID_CLIENT_PEER                             1155  /* "Client-Peer" */
#define YMO_HTTP_HID_CLIENT_RESPONSE_NUM                     9643  /* "Client-Response-Num" */
#define YMO_HTTP_HID_CONNECTION                             31919  /* "Connection" */
#define YMO_HTTP_HID_CONTENT_DISPOSITION                    20678  /* "Content-Disposition" */
#define YMO_HTTP_HID_CONTENT_ENCODING                       30996  /* "Content-Encoding" */
#define YMO_HTTP_HID_CONTENT_LANGUAGE                       13281  /* "Content-Language" */
#define YMO_HTTP_HID_CONTENT_LENGTH                         12647  /* "Content-Length" */
#define YMO_HTTP_HID_CONTENT_LOCATION                        6246  /* "Content-Location" */
#define YMO_HTTP_HID_CONTENT_MD5                            23393  /* "Content-MD5" */
#define YMO_HTTP_HID_CONTENT_RANGE                          25688  /* "Content-Range" */
#define YMO_HTTP_HID_CONTENT_SECURITY_POLICY                15382  /* "Content-Security-Policy" */
#define YMO_HTTP_HID_CONTENT_SECURITY_POLICY_REPORT_ONLY    19990  /* "Content-Security-Policy-Report-Only" */
#define YMO_HTTP_HID_CONTENT_TYPE                           22203  /* "Content-Type" */
#define YMO_HTTP_HID_COOKIE                                  7269  /* "Cookie" */
#define YMO_HTTP_HID_DNT                                    10145  /* "DNT" */
#define YMO_HTTP_HID_DATE                                   27707  /* "Date" */
#define YMO_HTTP_HID_DELTA_BASE                              4627  /* "Delta-Base" */
#define YMO_HTTP_HID_ETAG                                   26426  /* "ETag" */
#define YMO_HTTP_HID_EXPECT                                 20206  /* "Expect" */
#define YMO_HTTP_HID_EXPECT_CT                               3188  /* "Expect-CT" */
#define YMO_HTTP_HID_EXPIRES                                25883  /* "Expires" */
#define YMO_HTTP_HID_FIELD                                  26493  /* "Field" */
#define YMO_HTTP_HID_FORWARDED                               9023  /* "Forwarded" */
#define YMO_HTTP_HID_FROM                                   23787  /* "From" */
#define YMO_HTTP_HID_FRONT_END_HTTPS                        24116  /* "Front-End-Https" */
#define YMO_HTTP_HID_HTTP                                    1005  /* "HTTP" */
#define YMO_HTTP_HID_HTTP2_SETTINGS                          1895  /* "HTTP2-Settings" */
#define YMO_HTTP_HID_HOST                                   26265  /* "Host" */
#define YMO_HTTP_HID_IM                                     27965  /* "IM" */
#define YMO_HTTP_HID_IF_MATCH                               27778  /* "If-Match" */
#define YMO_HTTP_HID_IF_MODIFIED_SINCE                      11681  /* "If-Modified-Since" */
#define YMO_HTTP_HID_IF_NONE_MATCH                          27075  /* "If-None-Match" */
#define YMO_HTTP_HID_IF_RANGE                               25482  /* "If-Range" */
#define YMO_HTTP_HID_IF_UNMODIFIED_SINCE                     5398  /* "If-Unmodified-Since" */
#define YMO_HTTP_HID_KEEP_ALIVE                             14934  /* "Keep-Alive" */
#define YMO_HTTP_HID_LAST_MODIFIED                          16971  /* "Last-Modified" */
#define YMO_HTTP_HID_LINK                                   27519  /* "Link" */
#define YMO_HTTP_HID_LOCATION                                8770  /* "Location" */
#define YMO_HTTP_HID_MAX_FORWARDS                           29556  /* "Max-Forwards" */
#define YMO_HTTP_HID_NEL                                    22048  /* "NEL" */
#define YMO_HTTP_HID_ORIGIN                                 31375  /* "Origin" */
#define YMO_HTTP_HID_P3P                                     4240  /* "P3P" */
#define YMO_HTTP_HID_PERMISSIONS_POLICY                     23036  /* "Permissions-Policy" */
#define YMO_HTTP_HID_PRAGMA                                 26433  /* "Pragma" */
#define YMO_HTTP_HID_PREFER                                  1325  /* "Prefer" */
#define YMO_HTTP_HID_PREFERENCE_APPLIED                      6448  /* "Preference-Applied" */
#define YMO_HTTP_HID_PROXY_AUTHENTICATE                     19117  /* "Proxy-Authenticate" */
#define YMO_HTTP_HID_PROXY_AUTHORIZATION                     5837  /* "Proxy-Authorization" */
#define YMO_HTTP_HID_PROXY_CONNECTION                        3606  /* "Proxy-Connection" */
#define YMO_HTTP_HID_PUBLIC_KEY_PINS                         6631  /* "Public-Key-Pins" */
#define YMO_HTTP_HID_RANGE                                    460  /* "Range" */
#define YMO_HTTP_HID_REFERER                                 1000  /* "Referer" */
#define YMO_HTTP_HID_REFRESH                                 9946  /* "Refresh" */
#define YMO_HTTP_HID_REPORT_TO                               2987  /* "Report-To" */
#define YMO_HTTP_HID_RETRY_AFTER                            21570  /* "Retry-After" */
#define YMO_HTTP_HID_SAVE_DATA                              18729  /* "Save-Data" */
#define YMO_HTTP_HID_SEC_WEBSOCKET_EXTENSIONS               11183  /* "Sec-WebSocket-Extensions" */
#define YMO_HTTP_HID_SEC_WEBSOCKET_KEY                       1788  /* "Sec-WebSocket-Key" */
#define YMO_HTTP_HID_SEC_WEBSOCKET_PROTOCOL                 19175  /* "Sec-WebSocket-Protocol" */
#define YMO_HTTP_HID_SEC_WEBSOCKET_VERSION                  16357  /* "Sec-WebSocket-Version" */
#define YMO_HTTP_HID_SERVER                                 30792  /* "Server" */
#define YMO_HTTP_HID_SET_COOKIE                             10316  /* "Set-Cookie" */
#define YMO_HTTP_HID_STATUS                                  6483  /* "Status" */
#define YMO_HTTP_HID_STRICT_TRANSPORT_SECURITY               3375  /* "Strict-Transport-Security" */
#define YMO_HTTP_HID_TE                                     31070  /* "TE" */
#define YMO_HTTP_HID_TIMING_ALLOW_ORIGIN                    27524  /* "Timing-Allow-Origin" */
#define YMO_HTTP_HID_TK                                     31076  /* "Tk" */
#define YMO_HTTP_HID_TRAILER                                28074  /* "Trailer" */
#define YMO_HTTP_HID_TRANSFER_ENCODING                      15812  /* "Transfer-Encoding" */
#define YMO_HTTP_HID_UPGRADE                                17987  /* "Upgrade" */
#define YMO_HTTP_HID_UPGRADE_INSECURE_REQUESTS               8995  /* "Upgrade-Insecure-Requests" */
#define YMO_HTTP_HID_USER_AGENT                             16616  /* "User-Agent" */
#define YMO_HTTP_HID_VARY                                    6159  /* "Vary" */
#define YMO_HTTP_HID_VIA                                     8521  /* "Via" */
#define YMO_HTTP_HID_WWW_AUTHENTICATE                       32140  /* "WWW-Authenticate" */
#define YMO_HTTP_HID_WARNING                                18231  /* "Warning" */
#define YMO_HTTP_HID_X_ATT_DEVICEID                          8371  /* "X-ATT-DeviceId" */
#define YMO_HTTP_HID_X_ASPNET_VERSION                       15676  /* "X-Aspnet-Version" */
#define YMO_HTTP_HID_X_CONTENT_DURATION                      5654  /* "X-Content-Duration" */
#define YMO_HTTP_HID_X_CONTENT_SECURITY_POLICY               9965  /* "X-Content-Security-Policy" */
#define YMO_HTTP_HID_X_CONTENT_TYPE_OPTIONS                 27961  /* "X-Content-Type-Options" */
#define YMO_HTTP_HID_X_CORRELATION_ID                       13212  /* "X-Correlation-ID" */
#define YMO_HTTP_HID_X_CSRF_TOKEN                           27754  /* "X-Csrf-Token" */
#define YMO_HTTP_HID_X_FORWARDED_FOR                         5858  /* "X-Forwarded-For" */
#define YMO_HTTP_HID_X_FORWARDED_HOST                       32011  /* "X-Forwarded-Host" */
#define YMO_HTTP_HID_X_FORWARDED_PROTO                       8005  /* "X-Forwarded-Proto" */
#define YMO_HTTP_HID_X_FRAME_OPTIONS                        13216  /* "X-Frame-Options" */
#define YMO_HTTP_HID_X_HTTP_METHOD_OVERRIDE                 28957  /* "X-Http-Method-Override" */
#define YMO_HTTP_HID_X_PERMITTED_CROSS_DOMAIN_POLICIES      30637  /* "X-Permitted-Cross-Domain-Policies" */
#define YMO_HTTP_HID_X_PINGBACK                             32127  /* "X-Pingback" */
#define YMO_HTTP_HID_X_POWERED_BY                           14782  /* "X-Powered-By" */
#define YMO_HTTP_HID_X_REDIRECT_BY                          27466  /* "X-Redirect-By" */
#define YMO_HTTP_HID_X_REQUEST_ID                           24927  /* "X-Request-ID" */
#define YMO_HTTP_HID_X_REQUESTED_WITH                       29363  /* "X-Requested-With" */
#define YMO_HTTP_HID_X_ROBOTS_TAG                           30624  /* "X-Robots-Tag" */
#define YMO_HTTP_HID_X_UA_COMPATIBLE                        24207  /* "X-UA-Compatible" */
#define YMO_HTTP_HID_X_UIDH                                  9254  /* "X-UIDH" */
#define YMO_HTTP_HID_X_WAP_PROFILE                           5928  /* "X-Wap-Profile" */
#define YMO_HTTP_HID_X_WEBKIT_CSP                           20791  /* "X-WebKit-CSP" */
#define YMO_HTTP_HID_X_XSS_PROTECTION                         104  /* "X-XSS-Protection" */

#endif /* YMO_HTTP_HDR_TABLE_H */



