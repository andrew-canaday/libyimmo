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



/** Session
 * ====================
 * Types and functions to for individual WSGI sessions and their exchanges.
 *
 *
 */

#ifndef YMO_WSGI_SESSION_H
#define YMO_WSGI_SESSION_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ymo_http.h"

#include "yimmo_wsgi.h"
#include "ymo_wsgi_exchange.h"

/**---------------------------------
 *           Types
 *---------------------------------*/

/** This is where the python WSGI session is tracked C-side.
 */
struct ymo_wsgi_session {
    size_t                    id;
    ymo_wsgi_exchange_t*      head;
    ymo_wsgi_worker_t*        worker;
    pthread_mutex_t           lock;
    atomic_int_least16_t      refcnt;
    atomic_uint_least8_t      closed;
    ymo_wsgi_exchange_pool_t  pool;
};



/**---------------------------------
 *          Functions
 *---------------------------------*/

/** */
ymo_status_t ymo_wsgi_session_init(
        void* server_data, ymo_http_session_t* http_session);

/** */
void ymo_wsgi_session_cleanup(
        void* server_data, ymo_http_session_t* http_session, void* data);

/** */
ymo_wsgi_worker_t* ymo_wsgi_session_worker(ymo_wsgi_session_t* session);


/** */
ymo_wsgi_exchange_t* ymo_wsgi_session_create_exchange(
        ymo_wsgi_session_t* session,
        ymo_http_request_t* request,
        ymo_http_response_t* response
        );

/** */
void ymo_wsgi_session_exchange_done(ymo_wsgi_session_t* session);


/** */
size_t ymo_wsgi_session_incref(ymo_wsgi_session_t* session);

/** */
size_t ymo_wsgi_session_decref(ymo_wsgi_session_t* session);

/** */
void ymo_wsgi_session_close(ymo_wsgi_session_t* session);

/** */
int ymo_wsgi_session_is_closed(ymo_wsgi_session_t* session);

/** */
int ymo_wsgi_session_lock(ymo_wsgi_session_t* session);

/** */
int ymo_wsgi_session_trylock(ymo_wsgi_session_t* session);

/** */
int ymo_wsgi_session_unlock(ymo_wsgi_session_t* session);

#endif /* YMO_WSGI_SESSION_H */


