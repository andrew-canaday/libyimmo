/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 *  Copyright (c) 2014 Andrew Canaday
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

#include "ymo_config.h"

#include <stdlib.h>

#include "ymo_std_http.h"
#include "ymo_http_hdr_table.h"

const char* ymo_std_request_headers[] = {
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
    "Sec-WebSocket-Extensions",
    "Sec-WebSocket-Key",
    "Sec-WebSocket-Protocol",
    "Sec-WebSocket-Version",
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
};

const char* ymo_common_http_headers[] = {
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
    "Access-Control-Request-Method",
    "Age",
    "Allow",
    "Alt-Svc",
    "Alternate-Protocol",
    "Authorization",
    "Cache-Control",
    "Client-Date",
    "Client-Peer",
    "Client-Response-Num",
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
    "ETag",
    "Expect",
    "Expect-CT",
    "Expires",
    "Field",
    "Forwarded",
    "From",
    "Front-End-Https",
    "HTTP",
    "HTTP2-Settings",
    "Host",
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
    "Retry-After",
    "Save-Data",
    "Sec-WebSocket-Extensions",
    "Sec-WebSocket-Key",
    "Sec-WebSocket-Protocol",
    "Sec-WebSocket-Version",
    "Server",
    "Set-Cookie",
    "Status",
    "Strict-Transport-Security",
    "TE",
    "Timing-Allow-Origin",
    "Tk",
    "Trailer",
    "Transfer-Encoding",
    "Upgrade",
    "Upgrade-Insecure-Requests",
    "User-Agent",
    "Vary",
    "Via",
    "WWW-Authenticate",
    "Warning",
    "X-ATT-DeviceId",
    "X-Aspnet-Version",
    "X-Content-Duration",
    "X-Content-Security-Policy",
    "X-Content-Type-Options",
    "X-Correlation-ID",
    "X-Csrf-Token",
    "X-Forwarded-For",
    "X-Forwarded-Host",
    "X-Forwarded-Proto",
    "X-Frame-Options",
    "X-Http-Method-Override",
    "X-Permitted-Cross-Domain-Policies",
    "X-Pingback",
    "X-Powered-By",
    "X-Redirect-By",
    "X-Request-ID",
    "X-Requested-With",
    "X-Robots-Tag",
    "X-UA-Compatible",
    "X-UIDH",
    "X-Wap-Profile",
    "X-WebKit-CSP",
    "X-XSS-Protection",
};


const size_t ymo_no_std_request_headers = (
    sizeof(ymo_std_request_headers)/sizeof(const char*));
const size_t ymo_no_common_http_headers = (
    sizeof(ymo_common_http_headers)/sizeof(const char*));



