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



/** Main Process Functions
 * ========================
 *
 */

#ifndef YMO_WSGI_PROC_MAIN_H
#define YMO_WSGI_PROC_MAIN_H

/*---------------------------------*
 * Prototypes
 *---------------------------------*/

static void main_proc_init(ymo_wsgi_proc_t* w_proc);
static void ymo_wsgi_main_shutdown(
        ymo_wsgi_proc_t* w_proc, int my_pid);
static void ymo_wsgi_main_pkill(
        struct ev_loop* loop, ev_timer* w, int revents);
static int main_proc_teardown(ymo_wsgi_proc_t* w_proc);

/**---------------------------------
 * Functions
 *---------------------------------*/

/** Initialize signal handlers for the main process.
 *
 * Handles
 *
 * - ``SIGINT`` (being shutdown)
 * - ``SIGCHLD`` (restart child process)
 *
 */
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


/** Begin termination sequence for the WSGI server (typically, in response
 * to some signal).
 * */
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


/** Invoke by :c:func:`ymo_wsgi_main_shutdown` to terminate child processes
 * (if they exist).
 */
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


/** Cleans up main process resources on termination (i.e. when
 * :c:func:`ymo_wsgi_proc_main` completes).
 */
static int main_proc_teardown(ymo_wsgi_proc_t* w_proc)
{
    ymo_log_notice("Main process waiting on children.");
    int child_pid;
    while( (child_pid = (int)waitpid(-1, 0, 0)) > 0 )
    {
        ymo_log_notice("Child %i shut down", child_pid);
    }
    ymo_log_notice("Main process shutting down.");

    if( w_proc->cfg ) {
        ymo_yaml_doc_free(w_proc->cfg);
        w_proc->cfg = NULL;
    }
    return 0;
}


#endif /* YMO_WSGI_PROC_MAIN_H */


