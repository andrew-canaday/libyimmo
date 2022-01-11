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



/** Worker Process Functions
 * ==========================
 *
 */

#ifndef YMO_WSGI_PROC_WORKER_H
#define YMO_WSGI_PROC_WORKER_H

static int worker_proc_init(
        PyThreadState** _py_ts_save, ymo_wsgi_proc_t* w_proc);
static int worker_proc_teardown(
        PyThreadState** _py_ts_save, ymo_wsgi_proc_t* w_proc);
static void ymo_wsgi_worker_tkill(
        struct ev_loop* loop, ev_timer* w, int revents);
static void ymo_wsgi_worker_shutdown(
        ymo_wsgi_proc_t* w_proc, int my_pid);

/*---------------------------------*
 *      Worker Init/Shutdown:
 *---------------------------------*/

/** */
static void ymo_wsgi_worker_tkill(struct ev_loop* loop, ev_timer* w, int revents)
{
    ymo_wsgi_proc_t* w_proc = w->data;
    ymo_log_notice("Thread timer fired after %.3f seconds.",
            YMO_WSGI_TKILL_TIMEOUT);
    for( int thread_id = 0; thread_id < w_proc->no_wsgi_threads; thread_id++ ) {
        ymo_log_notice("Cancelling worker thread %i/%i",
                thread_id+1, w_proc->no_wsgi_threads);
        pthread_cancel(w_proc->w_threads[thread_id].thread);
        ymo_log_notice("Worker thread %i/%i cancelled.",
                thread_id+1, w_proc->no_wsgi_threads);
    }
}


/** */
static int worker_proc_init(
        PyThreadState** _py_ts_save, ymo_wsgi_proc_t* w_proc)
{
#if YMO_WSGI_REUSEPORT
    errno = 0;
    w_proc->http_srv = ymo_wsgi_server_init(
            w_proc->loop, w_proc->port, &w_proc);
    if( !w_proc->http_srv ) {
        ymo_log_fatal("Failed to create HTTP server: %s", strerror(errno));
        return -1;
    }
#endif /* YMO_WSGI_REUSEPORT */

    /* Initialize python interpretter and load w_proc->module: */
    if( ymo_wsgi_init(w_proc) != YMO_OKAY ) {
        return -1;
    }

    /* Save the current signal mask, then block SIGTERM and SIGINT: */
    sigset_t proc_mask;
    int r_val;
    if( (r_val = ymo_wsgi_signal_get_mask(&proc_mask)) ) {
        ymo_log_error("%i: error getting signal mask: %s",
                (int)getpid(), strerror(r_val));
    }
    ymo_wsgi_signal_mask(SIGINT, SIG_BLOCK);
    ymo_wsgi_signal_mask(SIGTERM, SIG_BLOCK);

    /* TODO: it's a bit hairy asking for an array of workers here... */
    /* Initialize the thread workers: */
    w_proc->w_threads = ymo_wsgi_init_workers(
            w_proc->no_wsgi_threads, w_proc->loop);

    /* Restore the signal mask and start the server: */
    ymo_wsgi_signal_set_mask(&proc_mask);
    ymo_wsgi_server_start(w_proc->loop);
    ymo_server_start(w_proc->http_srv, w_proc->loop);
    *_py_ts_save = PyEval_SaveThread();
    return 0;
}


/** */
static int worker_proc_teardown(
        PyThreadState** _py_ts_save, ymo_wsgi_proc_t* w_proc)
{
    PyEval_RestoreThread(*_py_ts_save);

    ymo_wsgi_stop_workers(w_proc->no_wsgi_threads, w_proc->w_threads);
    for( int thread_id = 0; thread_id < w_proc->no_wsgi_threads; thread_id++ ) {
        ymo_log_notice("Waiting on worker thread %i/%i",
                thread_id+1, w_proc->no_wsgi_threads);
        ymo_wsgi_worker_join(&(w_proc->w_threads[thread_id]));
        ymo_log_notice("Worker thread %i/%i done",
                thread_id+1, w_proc->no_wsgi_threads);
    }
    int rc = ymo_wsgi_shutdown();
    ymo_server_free(w_proc->http_srv);

    return rc;
}


/** */
static void ymo_wsgi_worker_shutdown(ymo_wsgi_proc_t* w_proc, int my_pid)
{
    /* Check to see if parent is dead. If so, something went wrong and
     * the user is probably trying to terminate the workers.
     * In this case, be polite to the user vs the clients.
     */
    if( 0 && waitpid(w_proc->ppid, 0, WNOHANG) <= 0 ) {
        ymo_log_notice("%i: parent process (%i) is dead. Exiting NOW.",
                my_pid, (int)w_proc->ppid);
        ev_break(w_proc->loop, EVBREAK_ALL);
        exit(0);
    }

    /* Otherwise, we'll try to shut down gracefully: */
    ymo_log_notice("%i got SIGTERM. Starting shutdown.", my_pid);
    ymo_server_stop_graceful(w_proc->http_srv);
    ymo_wsgi_stop_workers(w_proc->no_wsgi_threads, w_proc->w_threads);

    /* Start the kill timer: */
    ev_timer_init(&(w_proc->tkill_timer), &ymo_wsgi_worker_tkill,
            YMO_WSGI_TKILL_TIMEOUT, 0.0);
    ev_timer_start(w_proc->loop, &w_proc->tkill_timer);
    return;
}


#endif /* YMO_WSGI_PROC_WORKER_H */


