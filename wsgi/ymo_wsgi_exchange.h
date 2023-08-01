/*=============================================================================
 *
 * Copyright (c) 2021 Andrew Canaday
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



/** Exchange
 * ====================
 *
 * Types and functions to for individual exchanges (request/response pairs).
 *
 */

#ifndef YMO_WSGI_EXCHANGE_H
#define YMO_WSGI_EXCHANGE_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ymo_http.h"

#include "yimmo_wsgi.h"

/**---------------------------------
 *           Types
 *---------------------------------*/

/** Type used to store request/response pairs as an exchange.
 *
 */
struct ymo_wsgi_exchange {
    ymo_wsgi_session_t*       session;
    ymo_http_request_t*       request;
    ymo_http_response_t*      response;
    ymo_bucket_t*             body_data;
    struct ymo_wsgi_exchange* next;
    int                       done;
    int                       sent;
    int                       no_pool;   /* Gross */
    size_t                    body_read;
    atomic_int_least16_t      refcnt;
};


/** Pool of preallocated exchange objects for resuse.
 */
struct ymo_wsgi_exchange_pool {
    ymo_wsgi_exchange_t* head;
    ymo_wsgi_exchange_t  items[YMO_WSGI_EXCHANGE_POOL_SIZE];
};


/**---------------------------------
 *          Functions
 *---------------------------------*/

/** Create a new WSGI exchange to encapsulate a request/response pair for
 * the given session.
 *
 */
ymo_wsgi_exchange_t* ymo_wsgi_exchange_create(void);

void ymo_wsgi_exchange_init(
        ymo_wsgi_exchange_t* exchange,
        ymo_wsgi_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response
        );

/** Release a WSGI exchange.
 *
 */
void ymo_wsgi_exchange_free(
        ymo_wsgi_session_t* session, ymo_wsgi_exchange_t* exchange);

/** */
size_t ymo_wsgi_exchange_incref(ymo_wsgi_exchange_t* exchange);

/** */
size_t ymo_wsgi_exchange_decref(ymo_wsgi_exchange_t* exchange);

#define YIMMO_WSGI_TRACE_REFCNT 0

#if defined(YIMMO_WSGI_TRACE_REFCNT) && (YIMMO_WSGI_TRACE_REFCNT == 1)
#  define _c_l "\033[00;31;m"
#  define _c_r "\033[00;m"
#else
#  define _c_l
#  define _c_r
#endif


#if defined(YIMMO_WSGI_TRACE_REFCNT) && (YIMMO_WSGI_TRACE_REFCNT == 1)

#  define WSGI_EXCHANGE_INCREF(e) \
        ( \
            ymo_log_trace("\033[00;01;33;m%s:%i: ymo_wsgi_exchange_incref(%p)\033[00;m", __func__, __LINE__, (void*)e), \
            ymo_wsgi_exchange_incref(e) \
        )

#  define WSGI_EXCHANGE_DECREF(e) \
        ( \
            ymo_log_trace("\033[00;01;33;m%s:%i: ymo_wsgi_exchange_decref(%p)\033[00;m", __func__, __LINE__, (void*)e), \
            ymo_wsgi_exchange_decref(e) \
        )

#else

#  define WSGI_EXCHANGE_INCREF(e) \
        ymo_wsgi_exchange_incref(e)

#  define WSGI_EXCHANGE_DECREF(e) \
        ymo_wsgi_exchange_decref(e)

#endif /* YIMMO_WSGI_TRACE_REFCNT */

#endif /* YMO_WSGI_EXCHANGE_H */


