/*=============================================================================
 *
 *  Copyright (c) 2021 Andrew Canaday
 *
 *  This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
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


/** # ymo_wsgi_proc.c
 * Functions for controlling forking and signal handling in yimmo-wsgi
 *
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ev.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

#include "ymo_log.h"

#include "yimmo_wsgi.h"

#include "ymo_wsgi_mod.h"
#include "ymo_wsgi_cli.h"
#include "ymo_wsgi_server.h"
#include "ymo_wsgi_worker.h"
#include "ymo_wsgi_session.h"
#include "ymo_wsgi_proc.h"
#include "ymo_wsgi_util.h"


/* TODO: tidy tidy tidy! */
static void main_proc_init(ymo_wsgi_proc_t* w_proc);
static int main_proc_teardown(ymo_wsgi_proc_t* w_proc);
static int worker_proc_init(
        PyThreadState** _py_ts_save, ymo_wsgi_proc_t* w_proc);
static int worker_proc_teardown(
        PyThreadState** _py_ts_save, ymo_wsgi_proc_t* w_proc);
static void ymo_wsgi_main_pkill(
        struct ev_loop* loop, ev_timer* w, int revents);
static void ymo_wsgi_worker_tkill(
        struct ev_loop* loop, ev_timer* w, int revents);
static void ymo_wsgi_main_shutdown(
        ymo_wsgi_proc_t* w_proc, int my_pid);
static void ymo_wsgi_worker_shutdown(
        ymo_wsgi_proc_t* w_proc, int my_pid);
static void ymo_wsgi_sig_shutdown(
        ymo_wsgi_proc_t* w_proc, int w_sig);

/*---------------------------------*
 *        Process Workers:
 *---------------------------------*/
pid_t ymo_wsgi_proc_fork_new(ymo_wsgi_proc_t* w_proc, int worker_id)
{
    pid_t child_id = fork();
    if( child_id ) {
        return child_id;
    }

    /* Block SIGINT in child processes. */
    ymo_wsgi_signal_mask(SIGINT, SIG_BLOCK);
    ymo_wsgi_signal_mask(SIGCHLD, SIG_BLOCK);
#ifdef PROC_HAS_PPID
    unsigned int ppid = getppid();
    size_t len = snprintf(w_proc->proc_name, w_proc->proc_name_buflen, "yimmo-work-%i-%u",
            worker_id, ppid);
#else
    size_t len = snprintf(w_proc->proc_name, w_proc->proc_name_buflen, "yimmo-work-%i", worker_id);
#endif

    /* HACK: blank out the remainder of argv[0]... */
    for( size_t i = len; i < w_proc->proc_name_len; i++ ) {
        w_proc->proc_name[i] = ' ';
    }

    w_proc->ppid = getppid();
    w_proc->worker_id = worker_id;
    w_proc->is_worker = 1;
    w_proc->is_main = 0;
    // Notify the main w_proc->loop that we forked.
    ev_loop_fork(w_proc->loop);

    int exit_status = ymo_wsgi_proc_main(w_proc);
    exit(exit_status);

    return 0;
}


int ymo_wsgi_proc_main(ymo_wsgi_proc_t* w_proc)
{
    if( w_proc->is_main ) {
        main_proc_init(w_proc);
    }

    ev_signal_init(&(w_proc->sigterm_watcher), ymo_wsgi_proc_sigterm, SIGTERM);
    w_proc->sigterm_watcher.data = w_proc;
    ev_signal_start(w_proc->loop, &(w_proc->sigterm_watcher));

    PyThreadState* _py_ts_save;
    if( w_proc->is_worker && worker_proc_init(&_py_ts_save, w_proc) ) {
        return ymo_wsgi_shutdown();
    }

    ev_run(w_proc->loop,0);
    ymo_log(YMO_LOG_INFO, "Shutting down!");

    if( w_proc->is_worker ) {
        return worker_proc_teardown(&_py_ts_save, w_proc);
    }

    return main_proc_teardown(w_proc);
}


/*---------------------------------*
 *        Worker Utilities:
 *---------------------------------*/
ymo_wsgi_worker_t* ymo_wsgi_init_workers(int no_wsgi_threads, struct ev_loop* loop)
{
    /* Launch the thread: */
    ymo_wsgi_worker_t* workers = NULL;
    workers = calloc(sizeof(ymo_wsgi_worker_t), no_wsgi_threads);
    if( !workers ) {
        return NULL;
    }

    /* TODO: error check here... */
    for( int thread_id = 0; thread_id < no_wsgi_threads; thread_id++ ) {
        ymo_wsgi_worker_init(loop, thread_id, &workers[thread_id]);
        ymo_wsgi_worker_start(&workers[thread_id]);
    }

    return workers;
}


ymo_wsgi_worker_t* ymo_wsgi_proc_assign_worker(ymo_wsgi_proc_t* w_proc, size_t session_id)
{
    size_t worker_id = session_id % w_proc->no_wsgi_threads;
    YMO_WSGI_TRACE("Session %zu -> worker %zu (%p)",
            session_id, worker_id,
            &(w_proc->w_threads[worker_id]));
    return &(w_proc->w_threads[worker_id]);
}


/*---------------------------------*
 *       Main Init/Shutdown:
 *---------------------------------*/
static void ymo_wsgi_main_pkill(struct ev_loop* loop, ev_timer* w, int revents)
{
    int my_pid = (int)getpid();
    ymo_log_notice("Main %i kill timer fired after %.3f seconds.",
            my_pid, YMO_WSGI_PKILL_TIMEOUT);

    ev_break(loop, EVBREAK_ALL);
    if( w->data ) {
        ymo_wsgi_proc_t* w_proc = w->data;
        ymo_log_trace("Proc: %p", w->data);

        for( int i = 0; i < w_proc->no_wsgi_proc; i++ ) {
            ymo_log_notice(
                    "%i sending SIGTERM to %i", my_pid, (int)w_proc->children[i]);
            kill(w_proc->children[i], SIGKILL);
        }
    } else {
        ymo_log_debug("Got %p proc. Bailing.", w->data);
    }
}


static void main_proc_init(ymo_wsgi_proc_t* w_proc)
{
    /* Set up signal handlers: */
    ev_signal_init(&(w_proc->sigint_watcher), ymo_wsgi_proc_sigint, SIGINT);
    ev_signal_init(&(w_proc->sigchld_watcher), ymo_wsgi_proc_sigchld, SIGCHLD);
    w_proc->sigint_watcher.data = w_proc;
    w_proc->sigchld_watcher.data = w_proc;
    ev_signal_start(w_proc->loop, &(w_proc->sigint_watcher));
    ev_signal_start(w_proc->loop, &(w_proc->sigchld_watcher));
    return;
}


static int main_proc_teardown(ymo_wsgi_proc_t* w_proc)
{
    ymo_log_notice("Main process waiting on children.");
    int child_pid;
    while( (child_pid = (int)waitpid(-1, 0, 0)) > 0 )
    {
        ymo_log_notice("Child %i shut down", child_pid);
    }
    ymo_log_notice("Main process shutting down.");
    return 0;
}


/*---------------------------------*
 *      Worker Init/Shutdown:
 *---------------------------------*/
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


ymo_status_t ymo_wsgi_stop_workers(int no_wsgi_threads, ymo_wsgi_worker_t* workers)
{
    for( int thread_id = 0; thread_id < no_wsgi_threads; thread_id++ ) {
        ymo_log_notice("Stopping worker thread %i/%i",
                thread_id+1, no_wsgi_threads);
        workers[thread_id].running = 0;

        if( !pthread_mutex_trylock(&workers[thread_id].lock_in) ) {
            ymo_wsgi_worker_notify(&workers[thread_id]);
            ymo_wsgi_worker_unlock_in(&workers[thread_id]);
        }
        ymo_log_notice("Worker thread %i/%i notified.",
                thread_id+1, no_wsgi_threads);
    }

    return YMO_OKAY;
}


static void ymo_wsgi_main_shutdown(ymo_wsgi_proc_t* w_proc, int my_pid)
{
    ymo_log_notice(
            "%i: shutting down %i child processes",
            my_pid, w_proc->no_wsgi_proc);
    for( int i = 0; i < w_proc->no_wsgi_proc; i++ ) {
        ymo_log_notice(
                "%i sending SIGTERM to %i", my_pid, (int)w_proc->children[i]);
        kill(w_proc->children[i], SIGTERM);
    }

    /* Start the kill timer: */
    ev_timer_init(&(w_proc->pkill_timer), &ymo_wsgi_main_pkill,
            YMO_WSGI_PKILL_TIMEOUT, 0.0);
    ev_timer_start(w_proc->loop, &w_proc->pkill_timer);
}


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


static void ymo_wsgi_sig_shutdown(ymo_wsgi_proc_t* w_proc, int w_sig)
{
    int my_pid = (int)getpid();
    if( w_sig ) {
        ymo_log_notice(
                "Process %i got signal %s (%i)",
                my_pid, strsignal(w_sig), w_sig);
    }

    w_proc->term = 1;
    if( !w_proc->is_worker ) {
        ymo_wsgi_main_shutdown(w_proc, my_pid);
    } else {
        ymo_wsgi_worker_shutdown(w_proc, my_pid);
    }
}


/*---------------------------------*
 *        Signal Handling:
 *---------------------------------*/
void ymo_wsgi_proc_sigchld(struct ev_loop* loop, ev_signal* w, int revents)
{
    ymo_wsgi_proc_t* w_proc = w->data;

    if( w_proc && w_proc->term ) {
        ymo_log_notice("Child process terminated. Waiting on %zu more.",
                w_proc->no_wsgi_proc-1);
        if( --w_proc->no_wsgi_proc == 1 ) {
            ymo_log_notice("Main process (%i) terminating", getpid());
            ev_break(loop, EVBREAK_ALL);
        }
        return;
    }

    /* Stop SIGINT/SIGCHLD handlers: */
    ev_signal_stop(w_proc->loop, &(w_proc->sigint_watcher));
    ev_signal_stop(w_proc->loop, &(w_proc->sigchld_watcher));
    ev_signal_stop(w_proc->loop, &(w_proc->sigterm_watcher));

    int my_pid = (int)getpid();
    ymo_log_notice("Process %i got SIGCHLD", my_pid);

    int stat_loc = 0;
    int restart_limit = w_proc->no_wsgi_proc * YMO_WSGI_PROC_MAX_RESTARTS;
    if( !w_proc->is_worker ) {
        for( int i = 0; i < w_proc->no_wsgi_proc; i++ ) {
            ymo_log_notice("Checking child process %i", (int)w_proc->children[i]);
            if( waitpid(w_proc->children[i], &stat_loc, WNOHANG) ) {
                if( WIFEXITED(stat_loc) ) {
                    ymo_log_notice("Child process %i exited with status: %u",
                            (int)w_proc->children[i], WEXITSTATUS(stat_loc));
                } else if( WIFSIGNALED(stat_loc) ) {
                    ymo_log_notice(
                            "Child process %i terminated with signal: %u (%s)",
                            (int)w_proc->children[i], WTERMSIG(stat_loc),
                            strsignal(WTERMSIG(stat_loc)));
                } else {
                    ymo_log_notice("Child process %i no status",
                            (int)w_proc->children[i]);
                }

                if( w_proc->restart_count < YMO_WSGI_PROC_MAX_RESTARTS ) {
                    w_proc->restart_count++;
                    w_proc->children[i] = ymo_wsgi_proc_fork_new(w_proc, i+1);
                    ymo_log_notice("Respawned as %i (restarts %i/%i)",
                            (int)w_proc->children[i],
                            w_proc->restart_count,
                            restart_limit);
                } else {
                    ymo_log_error("Max restarts exceeded (%i). Terminating.",
                            w_proc->restart_count);
                    kill(getpid(), SIGTERM);
                }
            }
        }
    }

    /* Restart them in the main process: */
    ev_signal_start(w_proc->loop, &(w_proc->sigint_watcher));
    ev_signal_start(w_proc->loop, &(w_proc->sigchld_watcher));
    ev_signal_start(w_proc->loop, &(w_proc->sigterm_watcher));
    return;
}


void ymo_wsgi_proc_sigint(struct ev_loop* loop, ev_signal* w, int revents)
{
    ymo_wsgi_proc_t* w_proc = w->data;
    ymo_wsgi_sig_shutdown(w_proc, SIGINT);
    return;
}


void ymo_wsgi_proc_sigterm(struct ev_loop* loop, ev_signal* w, int revents)
{
    ymo_wsgi_proc_t* w_proc = w->data;
    ymo_wsgi_sig_shutdown(w_proc, SIGTERM);
    return;
}

