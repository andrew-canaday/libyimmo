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



/** Process Worker
 * =================
 *
 * Functions for controlling forking and signal handling in yimmo-wsgi
 *
 *
 */

#ifndef YMO_WSGI_PWORKER_H
# define YMO_WSGI_PWORKER_H
# include <ev.h>

#include "ymo_wsgi_worker.h"

/* Don't use SO_REUSEPORT on Darwin (connections are not distributed!) */
#if defined(_DARWIN_C_SOURCE) && (_DARWIN_C_SOURCE)
#define YMO_WSGI_REUSEPORT 0
#endif /* _C_DARWIN_C_SOURCE */

#ifndef YMO_WSGI_REUSEPORT
# if HAVE_DECL_SO_REUSEPORT
#  define YMO_WSGI_REUSEPORT 1
# else
#  define YMO_WSGI_REUSEPORT 0
# endif /* HAVE_DECL_SO_REUSEPORT */
#endif /* YMO_WSGI_REUSEPORT */

/**---------------------------------
 *           Types
 *---------------------------------*/

/** Stores process information for the WSGI server in both single- and
 * multi-processing mode.
 */
struct ymo_wsgi_proc {
    /* Process Info: */
    char*   proc_name;
    size_t  proc_name_len;
    size_t  proc_name_buflen;
    long    no_wsgi_proc;
    long    no_wsgi_threads;
    int     restart_count;
    long    port;

    /* Process Type: */
    int  is_main;
    int  is_worker;

    /* Main process: */
    pid_t*     children;
    ev_signal  sigint_watcher;
    ev_signal  sigchld_watcher;
    ev_signal  sigterm_watcher;

    /* Child process: */
    pid_t              ppid;
    int                worker_id;
    ymo_wsgi_worker_t* w_threads;

    /* All processes: */
    ymo_server_t*   http_srv;
    struct ev_loop* loop;
    char*           module;
    char*           app;
};


/**---------------------------------
 *          Functions
 *---------------------------------*/

/** Invoked by the main process in multi-processing mode to spawn a new
 * worker process.
 */
pid_t ymo_wsgi_proc_fork_new(ymo_wsgi_proc_t* w_proc, int worker_id);

/** Entrypoint for WSGI worker processes (or the main process in single-proc
 * mode).
 */
int ymo_wsgi_proc_main(ymo_wsgi_proc_t* w_proc);

/** SIGINT handler.
 */
void ymo_wsgi_proc_sigint(struct ev_loop* loop, ev_signal* w, int revents);

/** SIGCHLD handler.
 */
void ymo_wsgi_proc_sigchld(struct ev_loop* loop, ev_signal* w, int revents);

/** SIGTERM handler.
 */
void ymo_wsgi_proc_sigterm(struct ev_loop* loop, ev_signal* w, int revents);


/** Spawn ``no_wsgi_threads`` worker threads.
 *
 * Returns a pointer to an array of works.
 *
 * .. warning::
 *   This is a HACK.
 *
 */
ymo_wsgi_worker_t* ymo_wsgi_init_workers(
        int no_wsgi_threads, struct ev_loop* loop);


/** Stop the WSGI worker threads in the the ``workers`` array.
 *
 * .. warning::
 *   This is a HACK.
 */
ymo_status_t ymo_wsgi_stop_workers(
        int no_wsgi_threads, ymo_wsgi_worker_t* workers);


/** Given a session ID, return the worker that should handle requests from
 * the session.
 *
 */
ymo_wsgi_worker_t* ymo_wsgi_proc_assign_worker(ymo_wsgi_proc_t* w_proc, size_t session_id);

#endif /* YMO_WSGI_PWORKER_H */


