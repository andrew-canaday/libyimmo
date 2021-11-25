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



/** yimmo.h
 * =========
 *
 * Yimmo public API. Provides access to most commonly used types and routines.
 *
 * .. toctree::
 *   :hidden:
 *   :maxdepth: 3
 *
 * .. contents:: Contents
 *  :local:
 *  :depth: 3
 */

#ifndef YIMMO_H
#define YIMMO_H
#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <uuid/uuid.h>
#include <ev.h>

#include "ymo_config.h"

/*---------------------------------------------------------------*
 * Compiler Attributes wrappers:
 *---------------------------------------------------------------*/

#if defined(HAVE_VAR_ATTRIBUTE_MODE) && (HAVE_VAR_ATTRIBUTE_MODE == 1)
#define YMO_ATTR_MODE(x) __attribute__((mode(x)))
#define YMO_ENUM8_TYPEDEF(x) typedef enum x
#define YMO_ENUM8_AS(x) x YMO_ATTR_MODE(__byte__)
#define YMO_ENUM16_TYPEDEF(x) enum x
#define YMO_ENUM16_AS(x) ; typedef uint16_t x
// #define YMO_ENUM16_TYPEDEF(x) typedef enum x
// #define YMO_ENUM16_AS(x) x YMO_ATTR_MODE(HImode)
#else
#define YMO_ATTR_MODE(x)
#define YMO_ENUM8_TYPEDEF(x) enum x
#define YMO_ENUM8_AS(x) ; typedef uint8_t x
#define YMO_ENUM16_TYPEDEF(x) enum x
#define YMO_ENUM16_AS(x) ; typedef uint16_t x
#endif /* HAVE_VAR_ATTRIBUTE_MODE */

#if defined(HAVE_VAR_ATTRIBUTE_ALIGNED) && (HAVE_VAR_ATTRIBUTE_ALIGNED == 1)
#define YMO_ATTR_ALIGNED(x) __attribute__((aligned(x)))
#else
#define YMO_ATTR_ALIGNED(x)
#endif /* HAVE_VAR_ATTRIBUTE_ALIGNED */

#if defined(HAVE_FUNC_ATTRIBUTE_MALLOC) && (HAVE_FUNC_ATTRIBUTE_MALLOC == 1)
#define YMO_FUNC_ATTR_MALLOC __attribute__((malloc))
#else
#define YMO_FUNC_ATTR_MALLOC
#endif /* HAVE_FUNC_ATTRIBUTE_MALLOC */

#if defined(HAVE_FUNC_ATTRIBUTE_UNUSED) && (HAVE_FUNC_ATTRIBUTE_UNUSED == 1)
#define YMO_FUNC_ATTR_UNUSED __attribute__((unused))
#else
#define YMO_FUNC_ATTR_UNUSED
#endif /* HAVE_FUNC_ATTRIBUTE_UNUSED */

#if defined(HAVE_FUNC_ATTRIBUTE_FLATTEN) && (HAVE_FUNC_ATTRIBUTE_FLATTEN == 1)
#define YMO_FUNC_ATTR_FLATTEN __attribute__((flatten))
#else
#define YMO_FUNC_ATTR_FLATTEN
#endif /* HAVE_FUNC_ATTRIBUTE_FLATTEN */

#if defined(HAVE_FUNC_ATTRIBUTE_FALLTHROUGH) && (HAVE_FUNC_ATTRIBUTE_FALLTHROUGH == 1)
#define YMO_STMT_ATTR_FALLTHROUGH() __attribute__((fallthrough))
#else
#define YMO_STMT_ATTR_FALLTHROUGH()
#endif /* HAVE_FUNC_ATTRIBUTE_FALLTHROUGH */

#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) && (HAVE_FUNC_ATTRIBUTE_WEAK == 1)
#define YMO_FUNC_ATTR_WEAK __attribute__((weak))
#else
#define YMO_FUNC_ATTR_WEAK
#endif /* HAVE_FUNC_ATTRIBUTE_WEAK */

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/** Status code type used by the libyimmo API (generally follows errno.h). */
typedef int ymo_status_t;

/** Status code indicating everything went okay.
 *
 * At the moment, we use errno.h code definitions for other types of
 * error conditions.
 */
#define YMO_OKAY 0

/** Opaque struct used to represent a single server.
 *
 * A yimmo "server" consists of a listener for a *single port*.
 */
typedef struct ymo_server ymo_server_t;

/** Opaque struct used to represent a connection. */
typedef struct ymo_conn ymo_conn_t;

/** Opaque struct type used to represent a protocol. */
typedef struct ymo_proto ymo_proto_t;

/** Callback typedef for libev callbacks (for convenience).
 *
 * :param loop: ev_loop executing the callback
 * :param watcher: libev watcher type associated with the callback
 * :param revents: event mask used to indicate reason for the callback
 */
typedef void (* ymo_ev_cb_t)(
        struct ev_loop* loop,
        struct ev_io* watcher,
        int revents);

/** User-level connection init callback.
 *
 *
 * This function provides user-code with an opportunity to associate
 * application data with a newly initiated connection. The data associated
 * with the connection will also be passed to the user cleanup callback
 * just prior to final connection teardown.
 *
 * (see also :c:type:`ymo_user_conn_cleanup_cb_t`)
 *
 * :param server: server object issuing the callback
 * :param conn: new connection object
 *
 * Returns user data to associate with the new connection
 */
typedef void* (* ymo_user_conn_init_cb_t)(
        ymo_server_t* server,
        ymo_conn_t* conn);

/** User-level connection cleanup callback.
 *
 *
 * This function provides user-code with an opportunity to perform somea
 * cleanup on application data associated with the connection, just prior
 * to final teardown.
 *
 * (see also :c:type:`ymo_user_conn_init_cb_t`)
 *
 * :param server: server object issuing the callback
 * :param conn: new connection object
 * :param user: data associated with connection
 */
typedef void (* ymo_user_conn_cleanup_cb_t)(
        ymo_server_t* server,
        ymo_conn_t* conn,
        void* user);

/** Enumeration type used to pass server configuration flags.
 */
YMO_ENUM16_TYPEDEF(ymo_server_config_flags) {
    YMO_SERVER_REUSE_ADDR = 0x01, /* allow service to bind while socket in the WAIT state */
    YMO_SERVER_REUSE_PORT = 0x02, /* allow multiple processes to bind to the listen port */
} YMO_ENUM16_AS(ymo_server_config_flags_t);

/** Struct used to pass configuration information in at server create.
 */
typedef struct ymo_server_config {
    struct ev_loop*             loop;           /* I/O loop */
    ymo_user_conn_init_cb_t     user_init;      /* user conn init callback */
    ymo_user_conn_cleanup_cb_t  user_cleanup;   /* user conn cleanup callback */
    in_port_t                   port;           /* listen port */
    ymo_server_config_flags_t   flags;          /* server settings */
    int                         listen_backlog; /* TCP listen backlog */
} ymo_server_config_t;


/** Apache-esque "bucket" type used to handle streams of data. */
typedef struct ymo_bucket ymo_bucket_t;

typedef void (* ymo_bucket_free_fn)(ymo_bucket_t* bucket);


/**---------------------------------------------------------------
 * Library Functions
 *---------------------------------------------------------------*/

/** Get the current version as a string.
 *
 * Returns library version string
 */
const char* ymo_version_str(void);

/** Return the major version as an unsigned int. */
unsigned int ymo_version_major(void);

/** Return the minor version as an unsigned int. */
unsigned int ymo_version_minor(void);

/** Return the patch version as an unsigned int. */
unsigned int ymo_version_patch(void);

/** Return the extra version information. */
const char* ymo_version_extra(void);


/**---------------------------------------------------------------
 *  Buckets
 *---------------------------------------------------------------*/

/** Create a "bucket" containing data to process.
 *
 * A bucket has two pointer fields and two length fields. One points to the
 * data to be sent. The other is an optional pointer to the buffer containing
 * that data which is to freed on send.
 *
 * :param prev: a pointer to the previous bucket in the list (may be NULL)
 * :param next: a pointer to the next bucket in the list (may be NULL)
 * :param buf: an optional pointer to managed data to be freed on send (may be NULL)
 * :param buf_len: length of buf (must be specified if buf is non-NULL)
 * :param data: a pointer to the data to be sent
 * :param len: the length of the data to be sent
 * :returns: a new bucket created from the input parameters
 *
 * **Example**:
 *
 * ```C
 * // hello is not freed after send, because we just provide "data":
 * ymo_bucket_t* hello = ymo_bucket_create(
 *     NULL, NULL,    // don't bother with next/prev for first item
 *     NULL, 0,       // no "buf" set here
 *     "Hello, ", 7   // data to be sent is here
 *     );
 *
 * // world is freed after send because we provided the "data" (item to be
 * // sent), as well as "buf" (the underlying buffer):
 * char* world = strdup("World!");
 * ymo_bucket_t* world = ymo_bucket_create(
 *     hello, NULL,   // world is part of a chain; hello is before it
 *     world, 7,      // this is the buffer containing the payload
 *     world, 6       // this is the payload
 *     );
 *
 * // send "Hello, world!" somewhere:
 * some_send_function(fd, hello);
 * ```
 *
 */
ymo_bucket_t* ymo_bucket_create(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        char* buf, size_t buf_len, const char* data, size_t len);

/** Create a "bucket" copying data to process.
 *
 * :param prev: a pointer to the previous bucket in the list (may be NULL)
 * :param next: a pointer to the next bucket in the list (may be NULL)
 * :param buf: a pointer to the data to be copied and sent
 * :param buf_len: the length of the data to be copied sent
 * :returns: a new bucket created from the input parameters
 */
ymo_bucket_t* ymo_bucket_create_cpy(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        const char* buf, size_t buf_len);


/** Create a "bucket" from the file at `filepath`.
 *
 * .. warning::
 *    THIS IS A HACK!
 *
 * :param prev: a pointer to the previous bucket in the list (may be NULL)
 * :param next: a pointer to the next bucket in the list (may be NULL)
 * :param filepath: the filepath of the file data to be sent
 * :returns: a new bucket created from the input parameters
 *
 * :param prev: a pointer to the previous bucket in the list (may be NULL)
 * :param next: a pointer to the next bucket in the list (may be NULL)
 * :param filepath: the filepath of the file data to be sent
 * :returns: a new bucket created from the input parameters
 *
 */
ymo_bucket_t* ymo_bucket_from_file(
        ymo_bucket_t* restrict prev, ymo_bucket_t* restrict next,
        const char* filepath);

/** Create a bucket which references static data (or data that is
 * guaranteed to live longer than the bucket itself).
 *
 * :param data: a pointer to the data to be sent
 * :param len: the length of the data to be sent
 * :returns: a new bucket created from the input parameters
 */
#define YMO_BUCKET_FROM_REF(data, len) \
    ymo_bucket_create(NULL, NULL, NULL, 0, data, len)

/** Create a bucket by copying the given data and allowing yimmo
 * to manage the copy internally.
 *
 * :param data: a pointer to the data to be copied and sent
 * :param len: the length of the data to be copied and sent
 * :returns: a new bucket created from the input parameters
 */
#define YMO_BUCKET_FROM_CPY(data, len) \
    ymo_bucket_create_cpy(NULL, NULL, data, len)

/** Append src onto dst, if it exists; else return src.
 *
 * :param dst: Destination bucket.
 * :param src: Source bucket.
 * :returns: destination with src appended.
 */
ymo_bucket_t* ymo_bucket_append(
        ymo_bucket_t* restrict dst,
        ymo_bucket_t* restrict src);

/** Get the total length of a chain of buckets.
 *
 * :param p: head of bucket chain
 * :returns: length of all content in the chain of buckets
 */
size_t ymo_bucket_len_all(ymo_bucket_t* p);

/** Get the last bucket in a chain of buckets.
 *
 * :param p: a bucket in a chain of buckets
 * :returns: a pointers to the last bucket in the chain
 */
ymo_bucket_t* ymo_bucket_tail(ymo_bucket_t* p);

/** Free a bucket and - if managed - it's buffer.
 *
 * :param bucket: the bucket to free.
 */
void ymo_bucket_free(ymo_bucket_t* bucket);

/** Free all buckets in a chain.
 *
 * :param bucket: invoke :c:func:`ymo_bucket_free` on every bucket in a chain
 */
void ymo_bucket_free_all(ymo_bucket_t* bucket);


/**---------------------------------------------------------------
 *  Server Functions
 *---------------------------------------------------------------*/

/** Enum used to indicate state for :c:type:`ymo_server_t`. Used primarily
 * to track termination state (graceful vs hard stop, etc).
 */
typedef enum ymo_server_state {
    YMO_SERVER_CREATED,
    YMO_SERVER_INITIALIZED,
    YMO_SERVER_STARTED,
    YMO_SERVER_STOP_GRACEFUL,
} ymo_server_state_t;

/** Create a new server object.
 *
 * :param svr_config: configurations struct for the new server
 * :param proto: pointer to a protocol created using the init function in
 *        the appropriate module. (see also :c:type:`yimmo_modules`)
 * :returns: pointer to a new server instance or NULL on failure
 */
ymo_server_t* ymo_server_create(
        ymo_server_config_t* svr_config,
        ymo_proto_t* proto);

/** Bind the server to the configured port.
 *
 * :param server: the server to start
 * :returns: YMO_OKAY on success; appropriate errno on failure
 */
ymo_status_t ymo_server_init(ymo_server_t* server);

/** Create and start the ev_io watchers for the listen fd we invoke accept() on.
 *
 * :param server: the server to start
 * :returns: YMO_OKAY on success; appropriate errno on failure
 */
ymo_status_t ymo_server_start(ymo_server_t* server, struct ev_loop* loop);

/** */
ymo_server_state_t ymo_server_get_state(ymo_server_t* server);

/** Get the ev loop used by a given server. */
struct ev_loop* ymo_server_loop(ymo_server_t* server);

/** Gracefully shutdown a ymo_server.
 *
 * :param server: the server to stop
 * :returns: YMO_OKAY on success; appropriate errno on failure
 */
ymo_status_t ymo_server_stop_graceful(ymo_server_t* server);

/** Free a server object.
 *
 * :param server: the server instance to free
 */
void ymo_server_free(ymo_server_t* server);


/**---------------------------------------------------------------
 * Connection Functions
 *---------------------------------------------------------------*/

/** Given a connection, return a pointer to the owning server.
 *
 * :param conn: valid ymo_conn_t
 * :returns: the ymo_server_t instance which owns the given connection
 */
ymo_server_t* ymo_conn_server(const ymo_conn_t* conn);

/** Given a connection, return a pointer to the current protocol
 *
 * :param conn: valid ymo_conn_t
 * :returns: the current protocol for this connection
 */
ymo_proto_t* ymo_conn_proto(const ymo_conn_t* conn);

/** Given a connection, return its unique identifier.
 *
 * :param dst: the destination uuid_t object into which we copy the id
 * :param conn: valid ymo_conn_t
 */
void ymo_conn_id(uuid_t dst, const ymo_conn_t* conn);

/** Given a connection, return a pointer to the string representation of its
 * unique identifier.
 *
 * :param conn: a valid ymo_conn_t
 * :returns: `const char*` to string representation of conn uuid.
 *
 * .. warning:: This function is *not* thread-safe. The pointer returned is to
 *     a static string buffer. For multi-threaded uses, best leverage
 *     ymo_conn_id to copy the connection id into a local uuid_t and use
 *     uuid_unparse.
 *
 */
char* ymo_conn_id_str(const ymo_conn_t* conn);

#if defined(YMO_CONN_LOCK) && (YMO_CONN_LOCK == 1)
void ymo_conn_lock(ymo_conn_t* conn);
void ymo_conn_unlock(ymo_conn_t* conn);
#endif /* YMO_CONN_LOCK */


/**---------------------------------------------------------------
 * Protocol Functions
 *---------------------------------------------------------------*/

/** Given a protocol object, return the protocol name.
 *
 * :param proto: The protocol object we wish to identify
 * :returns: string representation of the protocol name
 */
const char* ymo_proto_name(ymo_proto_t* proto);


#endif /* YIMMO_H */




