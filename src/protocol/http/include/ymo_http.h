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




#ifndef YMO_HTTP_H
#define YMO_HTTP_H

#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <uuid/uuid.h>
#include <ev.h>
#include "yimmo.h"
#include "ymo_blalloc.h"

/** HTTP API
 * ==========
 *
 * This is the yimmo HTTP public API reference.
 *
 * .. admonition:: Info
 *
 *     To get a general sense for how the module is
 *     used, see :ref:`HTTP Overview`.
 *
 * .. contents:: Contents
 *   :local:
 *
 * .. toctree::
 *    :maxdepth: 2
 *    :hidden:
 *
 */

/* Protocol flags: */
#define YMO_HTTP_FLAG_VERSION_1_1       (1)
#define YMO_HTTP_FLAG_REQUEST_CLOSE     (0xff & (~(1<<2)))
#define YMO_HTTP_FLAG_REQUEST_KEEPALIVE (1<<2)
#define YMO_HTTP_FLAG_SUPPORTS_CHUNKED  (1<<3)
#define YMO_HTTP_FLAG_EXPECT            (1<<4)
#define YMO_HTTP_FLAG_UPGRADE           (1<<5)

/* Request flags: */
#define YMO_HTTP_REQUEST_CHUNKED        (1 << 16)

/* Response flags: */
#define YMO_HTTP_RESPONSE_READY         (1<<24)
#define YMO_HTTP_RESPONSE_STARTED       (1<<25)
#define YMO_HTTP_RESPONSE_COMPLETE      (1<<26)
#define YMO_HTTP_RESPONSE_CHUNKED       (1<<27)
#define YMO_HTTP_RESPONSE_CHUNK_TERM    (1<<28)


/**---------------------------------------------------------------
 * HTTP Headers
 *---------------------------------------------------------------*/

/**
 * .. note::
 *
 *    *Re: Set-Cookie:*
 *
 *    All HTTP response headers have to conform to the rules
 *    outlined in RFC 7230 (`section 3.2.2 — Field Order <RFC-7230-3.2.2>`_)
 *    *with the exception of the* ``Set-Cookie`` *header* which has its own
 *    grammar (outlined in RFC 6265, `section 4.1.1 — Set-Cookie Syntax <RFC-6265-4.1.1>`_).
 *
 *    At this time, the library uses a crude trick to accommodate this: it
 *    checks to see if the header being set is ``Set-Cookie`` and, if so, it
 *    adds another node to the header table, rather than concatenating the
 *    values.
 *
 *    This works out okay for *setting* cookies, but it also means
 *    that once a cookie has been inserted into a header table as part of
 *    *a single exchange*, it cannot be unset using either insert or add
 *    operations.
 *
 *    This **doesn't mean you can't unset cookies at all** — it *is possible*
 *    to unset a cookie by merely not setting it again on subsequent requests.
 *    You just can't set a cookie inside a request handler and then *unset* it
 *    in the *same response*.
 *
 *    **TL;DR**: The ``Set-Cookie`` header is essentially append-only.
 *
 *
 * .. warning::
 *
 *    *Re: storage:*
 *
 *    Generally, yimmo *does not copy header field names or data* (the only
 *    exception is concatenation — see :c:func:`ymo_http_hdr_table_add` for
 *    more info). The pointers you pass in are the locations in memory the
 *    header table will actually read from (but not write to — it does not
 *    modify any data passed in).
 *
 *    This means that you **are responsible for ensuring that the data
 *    referenced by the header name/value parameters exists at least as long
 *    as the yimmo response object**.
 *
 *    **Don't worry!**: there are facilities in the :c:struct:`ymo_http_request`
 *    type to facilitate exactly this kind of memory pattern (i.e. allocation
 *    arenas associated with a particular HTTP exchange) via the ``ws``
 *    (workspace) field.
 *
 *    For more info, see:
 *
 *    - :c:struct:`ymo_http_request`
 *    - :c:macro:`YMO_BLALLOC`, :c:func:`ymo_blalloc` and :c:func:`ymo_blalloc_strdup`
 */

/*  */
typedef uint32_t ymo_http_hdr_flags_t;

#define YMO_HDR_16_BIT
#ifdef YMO_HDR_16_BIT
typedef uint16_t ymo_http_hdr_id_t;
#else
typedef uint32_t ymo_http_hdr_id_t;
#endif /* YMO_HDR_16_BIT */

/**
 * HTTP header table type.
 */
typedef struct ymo_http_hdr_table ymo_http_hdr_table_t;

/** HTTP header table node type. */
typedef struct ymo_http_hdr_table_node ymo_http_hdr_table_node_t;

/** HTTP header table node pool type (when enabled). */
typedef struct ymo_http_hdr_table_pool ymo_http_hdr_table_pool_t;

/** HTTP header table node pointer — used by :c:func:`ymo_http_hdr_table_next`
 * as a cursor for iterating over tables.  */
typedef ymo_http_hdr_table_node_t* ymo_http_hdr_ptr_t;

/* HACK: return the header hash function override method as a string...
 *
 * (NOTE: I wouldn't use this. It's gonna disappear soon)
 *
 */
const char* ymo_http_hdr_hash_override_method(void);

/** Create a new HTTP header table.
 */
ymo_http_hdr_table_t* ymo_http_hdr_table_create(void);

/**
 * Insert a new value, ``value`` at key ``hdr``.
 *
 * .. warning::
 *    If the header name given by ``hdr`` is already present in
 *    the header table, it will be *replaced*.
 *
 * :param table: a pointer to the header table to modify.
 * :param hdr: the header field name.
 * :param value: the header field value.
 * :returns: the computed integer ID for the inserted header.
 */
ymo_http_hdr_id_t ymo_http_hdr_table_insert(
        ymo_http_hdr_table_t* table, const char* hdr, const char* value);

/**
 * Add the value, ``value`` at key ``hdr``.
 *
 * .. note::
 *    If the header name given by ``hdr`` is not present, it is created.
 *    If it's already present, the header values are joined using ``','`` and
 *    combined into a single header table entry.
 *
 * :param table: a pointer to the header table to modify.
 * :param hdr: the header field name.
 * :param value: the header field value.
 * :returns: the computed integer ID for the added header.
 */
ymo_http_hdr_id_t ymo_http_hdr_table_add(
        ymo_http_hdr_table_t* table, const char* hdr, const char* value);

/**
 * Get the value of a header field by name.
 *
 * :param table: a pointer to the header table to query
 * :param hdr: the name of the header field whose value we're fetching
 * :returns: a pointer to the *internal header field value* on success;
 *     ``NULL`` on failure.
 *
 */
const char* ymo_http_hdr_table_get(
        const ymo_http_hdr_table_t* table, const char* hdr);

/**
 * Iterate over all of the headers in a header table.
 *
 * :param table: a pointer to the header table to query
 * :param cur: a cursor used to traverse the header table.
 *     (This is set to ``NULL`` on the first invocation).
 * :param hdr: the header field *name* is stored here
 * :param hdr_len: the length of the header field *name* is stored here
 * :param value: the header field *value* is stored here
 * :returns: the next cursor or ``NULL`` once the end of the table is reached.
 *
 * .. code-block:: c
 *    :caption: Example: Print all of the entries in a header table
 *
 *    // These will be populated during iteration:
 *    const char* hdr_name;
 *    const char* hdr_value;
 *    size_t name_len;
 *
 *    // On the first invocation, we just
 *    // pass NULL for the cursor:
 *    ymo_http_hdr_ptr_t iter = ymo_http_hdr_table_next(
 *            table, NULL, &hdr_name, &name_len, &hdr_value);
 *
 *    // As long as the returned cursor is not
 *    // NULL, we've got data:
 *    while( iter )
 *    {
 *        printf("%.*s=%s\n", (int)name_len, hdr_name, hdr_value);
 *
 *        // Get the next entry in the table:
 *        iter = ymo_http_hdr_table_next(
 *                table, iter, &hdr_name, &name_len, &hdr_value);
 *    }
 *
 */
ymo_http_hdr_ptr_t ymo_http_hdr_table_next(
        ymo_http_hdr_table_t* table,
        ymo_http_hdr_ptr_t cur,
        const char** hdr,
        size_t* hdr_len,
        const char** value
        );


/** Remove all the entries from the hash table.
 */
void ymo_http_hdr_table_clear(ymo_http_hdr_table_t* table);

/** Free the given hash table.
 */
void ymo_http_hdr_table_free(ymo_http_hdr_table_t* table);

/*
 * .. todo::
 *   This should be adaptive (or at least configurable).
 */
#ifndef YMO_HDR_TABLE_BUCKET_SIZE
#  define YMO_HDR_TABLE_BUCKET_SIZE 8
#endif /* YMO_HDR_TABLE_BUCKET_SIZE */

#ifndef YMO_HDR_TABLE_POOL_SIZE
#  define YMO_HDR_TABLE_POOL_SIZE   4
#endif /* YMO_HDR_TABLE_POOL_SIZE */

#ifndef YMO_HDR_TABLE_MASK
#  define YMO_HDR_TABLE_MASK        0x7fff
#endif /* YMO_HDR_TABLE_MASK */


struct ymo_http_hdr_table_node {
    ymo_http_hdr_id_t          h_id;
    ymo_http_hdr_flags_t       flags;
    const char*                hdr;
    const char*                value;
    size_t                     hdr_len;
    ymo_http_hdr_table_node_t* next;
    char*                      buffer;
};

#if YMO_HDR_TABLE_POOL_SIZE
struct ymo_http_hdr_table_pool {
    ymo_http_hdr_table_node_t* head;
    ymo_http_hdr_table_node_t  items[YMO_HDR_TABLE_POOL_SIZE];
};
#endif /* YMO_HDR_TABLE_POOL_SIZE */

struct ymo_http_hdr_table {
    ymo_http_hdr_table_node_t* bucket[YMO_HDR_TABLE_BUCKET_SIZE];
#if YMO_HDR_TABLE_POOL_SIZE
    ymo_http_hdr_table_pool_t  pool;
#endif /* YMO_HDR_TABLE_POOL_SIZE */
};


/**---------------------------------------------------------------
 * Exchanges
 *---------------------------------------------------------------*/

/** Type Definitions
 * ..................
 */

/* Type used to store HTTP traits as binary flags.
 * HTTP flags are 4 bytes, laid out like so:
 *
 * .. code-block::
 *
 *    +---------------------------------------+
 *    |     Exchange Flags (bytes 0 and 1)    |
 *    +-------------------+-------------------+
 *    | 0 1 2 3 4 5 6 7 8 | 0 1 2 3 4 5 6 7 8 |
 *    +-------------------+-------------------+
 *    | V   K T E U       |                   |
 *    | 1   E E X P       |                   |
 *    | 1   E C P G       |                   |
 *    |     P H T D       |                   |
 *    +-------------------+-------------------+
 *    |  Request (byte 2) | Response (byte 3) |
 *    +-------------------+-------------------+
 *    | 0 1 2 3 4 5 6 7 8 | 0 1 2 3 4 5 6 7 8 |
 *    | B                 | R R R S C         |
 *    | T                 | D D D T T         |
 *    | E                 | R S C E Q         |
 *    | C                 |       C           |
 *    +-------------------+-------------------+
 *
 * Exhange:
 *
 * - ``V11``  HTTP Version 1.1 (1.0 assumed, otherwise)
 * - ``KEEP`` Session is keepalive (regardless of protocol version)
 * - ``TECH`` Session supports transfer-encoding: chunked
 *
 * Request:
 *
 * - ``BTEC`` Body Transfer-Encoding Chunked
 *
 * Response:
 *
 * - ``RDR``  Response data ready
 * - ``RDS``  Response data started
 * - ``RDC``  Response data complete
 * - ``STEC`` Send transfer-encoding chunked
 *
 */
typedef uint32_t ymo_http_flags_t;

/** Opaque struct used to represent HTTP requests.
 */
typedef struct ymo_http_request ymo_http_request_t;

/** Opaque struct used to represent HTTP responses.
 */
typedef struct ymo_http_response ymo_http_response_t;

/** Opaque struct used to represent HTTP requests-response pairs.
 */
typedef struct ymo_http_exchange ymo_http_exchange_t;


/** Opaque struct used to represent HTTP sessions.
 */
typedef struct ymo_http_session ymo_http_session_t;


/** Enum type used to indicate parsed HTTP method.
 */
YMO_ENUM8_TYPEDEF(ymo_http_method) {
    YMO_HTTP_METH_OPTIONS,
    YMO_HTTP_METH_GET,
    YMO_HTTP_METH_HEAD,
    YMO_HTTP_METH_POST,
    YMO_HTTP_METH_PUT,
    YMO_HTTP_METH_DELETE,
    YMO_HTTP_METH_TRACE,
    YMO_HTTP_METH_CONNECT,
    YMO_HTTP_METH_OTHER,
} YMO_ENUM8_AS(ymo_http_method_t);

/** Convenience enums for HTTP status codes.
 *
 */
typedef uint16_t ymo_http_status_t;


/** Requests
 * ..........
 */

/** HTTP request callback structure.
 *
 * .. note::
 *
 *    *Re: request "Workspaces"*
 *
 *    Yimmo provides a Varnish-style, per-request, block-memory, workspace which
 *    can be used to acquire memory on demand which is automatically freed after
 *    the request has completed (or otherwise been disposed of), e.g.:
 *
 *    .. code-block:: c
 *
 *       // Contrived example: set the HTTP header
 *       // "hex-counter" to the value of some global
 *       // counter, incremented with each request:
 *       typedef ymo_status_t my_http_callback(
 *               ymo_http_session_t* session,
 *               ymo_http_request_t* request,
 *               ymo_http_response_t* response,
 *               void* user_data)
 *       {
 *          // Allocate space enough for "0x" + 4 hex digits + '\0' terminal:
 *          char* value = ymo_blalloc(request->ws, alignof(char), 7);
 *          snprintf(value, "0x%x", ++my_global_counter, 7);
 *
 *          ymo_http_hdr_table_insert(&request->headers, "hex-counter", value);
 *
 *          // ... do other request handling stuff ...
 *          ymo_http_response_finish(response);
 *          return YMO_OKAY;
 *       }
 *
 *    The workspace object, ``ws`` is allocated and deallocated by yimmo
 *    automatically — i.e. ``ws`` is already usable at the time that
 *    any of the HTTP callbacks are invoked. No :c:func:`ymo_blalloc_create` or
 *    :c:func:`ymo_blalloc_free` invocations are required!
 */
struct ymo_http_request {
    const char*           method;           /* GET, POST, etc */
    const char*           uri;              /* Full URI, *as received* */
    const char*           version;          /* Protocol version *string* */
    const char*           query;            /* Query string, *as received* */
    const char*           fragment;         /* URI fragment, *as received* */
    ymo_http_hdr_table_t  headers;          /* Request headers */
    ymo_blalloc_t*        ws;               /* Request workspace */
    char*                 body;             /* Optionally buffered body data */
    size_t                body_received;    /* Body data received */
    size_t                content_length;   /* Content-length, per client */
    ymo_http_flags_t      flags;            /* Request flags */
    void*                 user;             /* Arbitrary, per-request, user-data */
};

/** Responses
 * ...........
 */

/** Get the header table for a response object. */
ymo_http_hdr_table_t* ymo_http_response_get_headers(
        ymo_http_response_t* response);

/** Get the value of the header given by ``hdr_name``.
 *
 * :param response: the HTTP response object to modify
 * :param header: the HTTP header to get
 *
 */
const char* ymo_http_response_get_header(
        const ymo_http_response_t* response, const char* hdr_name);

/** Used to set HTTP header values for responses
 *
 * :param response: the HTTP response object to modify
 * :param header: the HTTP header to set
 * :param value: the value for the header
 *
 * :returns: YMO_OKAY on success; appropriate errno on failure
 */
ymo_status_t ymo_http_response_add_header(
        ymo_http_response_t* response, const char* header, const char* value);

/** Used to set HTTP header values for responses
 *
 * :param response: the HTTP response object to modify
 * :param header: the HTTP header to set
 * :param value: the value for the header
 *
 * :returns: YMO_OKAY on success; appropriate errno on failure
 */
ymo_status_t ymo_http_response_insert_header(
        ymo_http_response_t* response, const char* header, const char* value);

/** Add HTTP body data
 *
 * :param response: the HTTP response object to which we append body data
 * :param body_data: the body payload as a :c:type:`ymo_bucket_t` (see :ref:`Yimmo Buckets` for more info)
 *
 */
void ymo_http_response_body_append(
        ymo_http_response_t* response, ymo_bucket_t* body_data);

/** Set the HTTP status code — the reason will be filled in from a table
 * or else "NULL".
 *
 * :param response: the HTTP response object to modify
 * :param status: the integer HTTP status code
 */
void ymo_http_response_set_status(
        ymo_http_response_t* response, ymo_http_status_t status);

/** Set the HTTP status code and reason as a string.
 *
 * :param response: the HTTP response object to modify
 * :param status: the integer HTTP status code
 */
int ymo_http_response_set_status_str(
        ymo_http_response_t* response, const char* status_str);

/* Internal only, at the moment + schedule for refactor. */
ymo_status_t ymo_http_response_issue(
        ymo_http_response_t* response, ymo_http_status_t status);

/* Set HTTP response flags (internal only + subject to refactor).
 *
 * :param response: the HTTP response object to modify
 * :param flags: HTTP response traits
 */
void ymo_http_response_set_flags(
        ymo_http_response_t* response, ymo_http_flags_t flags);

/** Mark the http response as ready for transmission.
 */
void ymo_http_response_ready(ymo_http_response_t* response);

/** :returns: true if this response has data ready to send; false otherwise.
 */
int ymo_http_response_is_ready(const ymo_http_response_t* response);

/** Mark the response data as complete (i.e. fully ready to transmit)
 */
void ymo_http_response_finish(ymo_http_response_t* response);

/** :returns: true if this response is finished; false otherwise.
 */
int ymo_http_response_finished(const ymo_http_response_t* response);


/**---------------------------------------------------------------
 * Sessions
 *---------------------------------------------------------------*/

/** Set session-level user data.
 */
void ymo_http_session_set_userdata(ymo_http_session_t* session, void* data);

/** Get session-level user data.
 */
void* ymo_http_session_get_userdata(const ymo_http_session_t* session);


/**---------------------------------------------------------------
 * Callbacks
 *---------------------------------------------------------------*/

/**
 * .. note::
 *
 *    See :ref:`HTTP Callbacks` for more info.
 *
 */

/** Session initialization callback.
 */
typedef ymo_status_t (*ymo_http_session_init_cb_t)(
        void* server_data, ymo_http_session_t* session);

/** Session cleanupialization callback.
 */
typedef void (*ymo_http_session_cleanup_cb_t)(
        void* server_data, ymo_http_session_t* session, void* user_data);


/** HTTP exchange callback.
 *
 * :param session: The client HTTP session
 * :param exchange: The originating exchange object
 * :param response: A blank response object
 * :param user_data: user data if set; else ``NULL``
 *
 * :returns: YMO_OKAY on success; errno.h value on failure.
 */
typedef ymo_status_t (*ymo_http_cb_t)(
        ymo_http_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        void* user_data);

/** HTTP exchange callback.
 *
 * :param session: The client HTTP session
 * :param exchange: The originating exchange object
 * :param response: A blank response object
 * :param user: user data if set; else ``NULL``
 *
 * :returns: YMO_OKAY on success; errno.h value on failure.
 */
typedef ymo_http_cb_t ymo_http_header_cb_t;

/** HTTP exchange callback.
 *
 * :param session: the client HTTP session
 * :param exchange: the originating exchange object
 * :param data: exchange body data
 * :param len: size of body data in bytes
 * :param user_data: user data if set; else ``NULL``
 *
 */
typedef ymo_status_t (*ymo_http_body_cb_t)(
        ymo_http_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response,
        const char* data,
        size_t len,
        void* user_data);

/**---------------------------------------------------------------
 * Protocol Management
 *---------------------------------------------------------------*/

/** */
YMO_ENUM8_TYPEDEF(ymo_http_upgrade_status) {
    YMO_HTTP_UPGRADE_HANDLED,
    YMO_HTTP_UPGRADE_IGNORE,
    YMO_HTTP_UPGRADE_ERROR,
    YMO_HTTP_UPGRADE_NOPROTO,
} YMO_ENUM8_AS(ymo_http_upgrade_status_t);

/** HTTP upgrade handler callback.
 *
 * :param hdr_upgrade: the value of the HTTP "Upgrade:" header
 * :param exchange: the originating http exchange object
 * :param response: a blank response object
 *
 * :returns: ymo_http_status_t indicator + set errno on failure
 */
typedef ymo_http_upgrade_status_t (*ymo_http_upgrade_cb_t)(
        const char* hdr_upgrade,
        ymo_http_request_t* request,
        ymo_http_response_t* response);

/** Struct that encapsulates functions and data required to validate and
 * perform and HTTP Upgrade exchange protocol switch.
 */
typedef struct ymo_http_upgrade_handler {
    ymo_http_upgrade_cb_t  cb;
    ymo_proto_t*           proto_new;
} ymo_http_upgrade_handler_t;


/** Yimmo has no HTTP/2 support, at the moment. This callback can be used to
 * reject an upgrade exchange for h2c over HTTP/1.0 or HTTP/1.1.
 */
ymo_http_upgrade_handler_t* ymo_http2_no_upgrade_handler(void);

/** Used to create a new HTTP protocol endpoint.
 */
ymo_proto_t* ymo_proto_http_create(
        ymo_http_session_init_cb_t session_init,
        ymo_http_cb_t http_callback,
        ymo_http_header_cb_t header_callback,
        ymo_http_body_cb_t body_cb,
        ymo_http_session_cleanup_cb_t session_cleanup,
        void* data);

/** Used to add an upgrade handler to the internal upgrade handler chain.
 */
ymo_status_t ymo_http_add_upgrade_handler(
        ymo_proto_t* proto,
        ymo_http_upgrade_handler_t* upgrade_handler);


/**---------------------------------------------------------------
 * Quickstart
 *---------------------------------------------------------------*/

/** Create a server by
 *
 * - invoking :c:func:`ymo_proto_http_create` to create a new, buffered, HTTP
 *     protocol object with the given port and handler callback.
 * - passing that proto object to :c:func:`ymo_server_create`, along with the
 *     default :c:type:`ymo_serfer_config_t`.
 *
 * :param loop: pointer to a libev loop object, or NULL for default
 * :param port: port number to bind to
 * :param http_callback: buffered HTTP callback handler
 * :param upgrade_handlers: an array of :c:type:`ymo_http_upgrade_handler_t`
 *     pointers for HTTP upgrade requests.
 * :param data: arbitrary pointer to associate with this server (available
 *     in callbacks as ``data``).
 * :returns: a :c:type:`ymo_server_t` pointer on success; ``NULL`` with
 *     ``errno`` set on failure.
 */
ymo_server_t* ymo_http_simple_init(
        struct ev_loop* loop,
        in_port_t port,
        ymo_http_cb_t http_callback,
        ymo_http_upgrade_handler_t** upgrade_handlers,
        void* data
        );

/**---------------------------------------------------------------
 * Misc
 *---------------------------------------------------------------*/

/** HTTP Status 100 (Continue)
 */
#define YMO_HTTP_CONTINUE                          100

/** HTTP Status 101 (Switching protocols)
 */
#define YMO_HTTP_SWITCHING_PROTOCOLS               101

/** HTTP Status 200 (Ok)
 */
#define YMO_HTTP_OK                                200

/** HTTP Status 201 (Created)
 */
#define YMO_HTTP_CREATED                           201

/** HTTP Status 202 (Accepted)
 */
#define YMO_HTTP_ACCEPTED                          202

/** HTTP Status 203 (Non authoritative information)
 */
#define YMO_HTTP_NON_AUTHORITATIVE_INFORMATION     203

/** HTTP Status 204 (No content)
 */
#define YMO_HTTP_NO_CONTENT                        204

/** HTTP Status 205 (Reset content)
 */
#define YMO_HTTP_RESET_CONTENT                     205

/** HTTP Status 206 (Partial content)
 */
#define YMO_HTTP_PARTIAL_CONTENT                   206

/** HTTP Status 300 (Multiple choices)
 */
#define YMO_HTTP_MULTIPLE_CHOICES                  300

/** HTTP Status 301 (Moved permanently)
 */
#define YMO_HTTP_MOVED_PERMANENTLY                 301

/** HTTP Status 302 (Found)
 */
#define YMO_HTTP_FOUND                             302

/** HTTP Status 303 (See other)
 */
#define YMO_HTTP_SEE_OTHER                         303

/** HTTP Status 304 (Not modified)
 */
#define YMO_HTTP_NOT_MODIFIED                      304

/** HTTP Status 305 (Use proxy)
 */
#define YMO_HTTP_USE_PROXY                         305

/** HTTP Status 307 (Temporary redirect)
 */
#define YMO_HTTP_TEMPORARY_REDIRECT                307

/** HTTP Status 400 (Bad request)
 */
#define YMO_HTTP_BAD_REQUEST                       400

/** HTTP Status 401 (Unauthorized)
 */
#define YMO_HTTP_UNAUTHORIZED                      401

/** HTTP Status 402 (Payment required)
 */
#define YMO_HTTP_PAYMENT_REQUIRED                  402

/** HTTP Status 403 (Forbidden)
 */
#define YMO_HTTP_FORBIDDEN                         403

/** HTTP Status 404 (Not found)
 */
#define YMO_HTTP_NOT_FOUND                         404

/** HTTP Status 405 (Method not allowed)
 */
#define YMO_HTTP_METHOD_NOT_ALLOWED                405

/** HTTP Status 406 (Not acceptable)
 */
#define YMO_HTTP_NOT_ACCEPTABLE                    406

/** HTTP Status 407 (Proxy authentication required)
 */
#define YMO_HTTP_PROXY_AUTHENTICATION_REQUIRED     407

/** HTTP Status 408 (Request timeout)
 */
#define YMO_HTTP_REQUEST_TIMEOUT                   408

/** HTTP Status 409 (Conflict)
 */
#define YMO_HTTP_CONFLICT                          409

/** HTTP Status 410 (Gone)
 */
#define YMO_HTTP_GONE                              410

/** HTTP Status 411 (Length required)
 */
#define YMO_HTTP_LENGTH_REQUIRED                   411

/** HTTP Status 412 (Precondition failed)
 */
#define YMO_HTTP_PRECONDITION_FAILED               412

/** HTTP Status 413 (Request entity too large)
 */
#define YMO_HTTP_REQUEST_ENTITY_TOO_LARGE          413

/** HTTP Status 414 (Request uri too long)
 */
#define YMO_HTTP_REQUEST_URI_TOO_LONG              414

/** HTTP Status 415 (Unsupported media type)
 */
#define YMO_HTTP_UNSUPPORTED_MEDIA_TYPE            415

/** HTTP Status 416 (Requested range not satisfiable)
 */
#define YMO_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE   416

/** HTTP Status 417 (Expectation failed)
 */
#define YMO_HTTP_EXPECTATION_FAILED                417

/** HTTP Status 426 (Upgrade required)
 */
#define YMO_HTTP_UPGRADE_REQUIRED                  426

/** HTTP Status 428 (Precondition required)
 */
#define YMO_HTTP_PRECONDITION_REQUIRED             428

/** HTTP Status 429 (Too many requestes)
 */
#define YMO_HTTP_TOO_MANY_REQUESTES                429

/** HTTP Status 431 (Request header fields too large)
 */
#define YMO_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE   431

/** HTTP Status 500 (Internal server error)
 */
#define YMO_HTTP_INTERNAL_SERVER_ERROR             500

/** HTTP Status 501 (Not implemented)
 */
#define YMO_HTTP_NOT_IMPLEMENTED                   501

/** HTTP Status 502 (Bad gateway)
 */
#define YMO_HTTP_BAD_GATEWAY                       502

/** HTTP Status 503 (Service unavailable)
 */
#define YMO_HTTP_SERVICE_UNAVAILABLE               503

/** HTTP Status 504 (Gateway timeout)
 */
#define YMO_HTTP_GATEWAY_TIMEOUT                   504

/** HTTP Status 505 (Http version not supported)
 */
#define YMO_HTTP_HTTP_VERSION_NOT_SUPPORTED        505

/** Given an array of three or more ``char`` from a **trusted source**,
 * convert the character string to an integer.
 *
 * .. warning::
 *    There is no:
 *
 *    - bounds checking
 *    - range checking
 *    - validation
 */
#define YMO_HTTP_STATUS_CHR3_TO_INT(s) \
    ( \
        ((s[0] & 0xf) * 100) + \
        ((s[1] & 0xf) * 10) + \
        (s[2] & 0xf) \
    )

#endif /* YMO_HTTP_H */

