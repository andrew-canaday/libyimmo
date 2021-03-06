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


/** # main.c
 * Yimmo WSGI server.
 *
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ev.h>
#include <stdio.h>
#include <libgen.h>

#include "ymo_log.h"
#include "ymo_env.h"

#include "yimmo_wsgi.h"

#include "ymo_wsgi_mod.h"
#include "ymo_wsgi_cli.h"
#include "ymo_wsgi_server.h"
#include "ymo_wsgi_worker.h"
#include "ymo_wsgi_session.h"
#include "ymo_wsgi_proc.h"
#include "ymo_wsgi_util.h"


/*---------------------------------*
 *           Prototypes:
 *---------------------------------*/
static int get_module_app(ymo_wsgi_proc_t* w_proc, int argc, char** argv);
static void config_from_env(ymo_wsgi_proc_t* w_proc);
static struct ev_loop* get_ev_default_loop(void);
static int init_http_server(ymo_wsgi_proc_t* proc);


/*---------------------------------*
 *            Globals:
 *---------------------------------*/
const char* script_name = NULL;

static ymo_wsgi_proc_t w_proc = {
    .proc_name = NULL,
    .proc_name_len = 0,
    .proc_name_buflen = 0,
    .no_wsgi_proc = YIMMO_WSGI_NO_PROC,
    .is_main = 1,
    .is_worker = 0,
    .children = NULL,
    .worker_id = 0,
    .http_srv = NULL,
    .loop = NULL,
    .module = NULL,
    .app = NULL,
    .restart_count = 0
};


/*---------------------------------*
 *             Main:
 *---------------------------------*/
int main(int argc, char** argv)
{
    /* HACK: make stderr line-buffered for log output. */
    setvbuf(stderr, NULL, _IOLBF, 0);

    ymo_log_init();

    /* HACK: stash process name for worker adjustment later: */
    w_proc.proc_name = argv[0];
    w_proc.proc_name_len = strlen(w_proc.proc_name);
    w_proc.proc_name_buflen = w_proc.proc_name_len+1;

    w_proc.module = getenv("YIMMO_WSGI_MODULE");
    w_proc.app = getenv("YIMMO_WSGI_APP");

    if( !w_proc.module || !w_proc.app ) {
        errno = 0;
        if( get_module_app(&w_proc, argc, argv) ) {
            fprintf(stderr, "Unable to load module: %s", strerror(errno));
            return -1;
        }
    }

    script_name = basename(w_proc.module);

    /* Say hello */
    issue_startup_msg(&w_proc);
    config_from_env(&w_proc);
    ymo_log_notice("WSGI app: %s", w_proc.app);

    /* libev w_proc.loop init: */
    w_proc.loop = get_ev_default_loop();

    /* Main process wants SIGINT, SIGCHLD, and SIGTERM: */
    ymo_wsgi_signal_mask(SIGINT, SIG_UNBLOCK);
    ymo_wsgi_signal_mask(SIGCHLD, SIG_UNBLOCK);
    ymo_wsgi_signal_mask(SIGTERM, SIG_UNBLOCK);

    /* Initialize the server: */
    if( init_http_server(&w_proc) ) {
        return -1;
    }

    if( w_proc.no_wsgi_proc > 1 ) {
        w_proc.children = calloc(w_proc.no_wsgi_proc, sizeof(pid_t));
        if( !w_proc.children ) {
            ymo_log_fatal("Failed to allocate memory for WSGI workers: %s",
                    strerror(errno));
            return -1;
        }
    } else {
        w_proc.is_worker = 1;
        return ymo_wsgi_proc_main(&w_proc);
    }

    /* TODO: move this elsewhere: */
    for( int i = 1; i <= w_proc.no_wsgi_proc; ++i ) {
        ymo_log_notice("Starting process %i/%i", i, w_proc.no_wsgi_proc);
        w_proc.children[i-1] = ymo_wsgi_proc_fork_new(&w_proc, i);
    }

    return ymo_wsgi_proc_main(&w_proc);
}


static void config_from_env(ymo_wsgi_proc_t* w_proc)
{
    /* Get the port from the env: */
    long default_port = DEFAULT_HTTP_PORT;
    ymo_env_as_long("YIMMO_WSGI_PORT", &w_proc->port, &default_port);

    /* Get process configuration from env: */
    w_proc->no_wsgi_proc = YIMMO_WSGI_NO_PROC;
    ymo_env_as_long("YIMMO_WSGI_NO_PROC", &w_proc->no_wsgi_proc, NULL);

    if( w_proc->no_wsgi_proc < 1 ) {
        w_proc->no_wsgi_proc = YIMMO_WSGI_NO_PROC;
    }
    ymo_log_notice("Number of WSGI processes: %i", w_proc->no_wsgi_proc);

    /* Get thread configuration from env: */
    w_proc->no_wsgi_threads = YIMMO_WSGI_NO_THREADS;
    ymo_env_as_long("YIMMO_WSGI_NO_THREADS", &w_proc->no_wsgi_threads, NULL);
    if( w_proc->no_wsgi_threads < 1 ) {
        w_proc->no_wsgi_threads = YIMMO_WSGI_NO_THREADS;
    }
    ymo_log_notice("Number of WSGI threads: %i", w_proc->no_wsgi_threads);
    return;
}


static struct ev_loop* get_ev_default_loop(void)
{
#if defined(HAVE_EPOLL_CTL) && (HAVE_EPOLL_CTL == 1)
    return ev_default_loop(EVBACKEND_EPOLL);
#else
    /* NOTE: KQUEUE is not recommended on Darwin.
     * ALSO: KQUEUE is slower than select on Darwin.
     * ALSO: KQUEUE crashes if concurrent fd's get too high...
     * struct ev_loop* loop = ev_default_loop(EVBACKEND_KQUEUE);
     */
# ifdef YIMMO_WSGI_USE_KQUEUE
    return ev_default_loop(EVBACKEND_KQUEUE);
# else
    return ev_default_loop(0);
# endif /* YIMMO_WSGI_USE_KQUEUE */
#endif /* HAVE_EPOLL_CTL */
}


static int init_http_server(ymo_wsgi_proc_t* w_proc)
{
#if !YMO_WSGI_REUSEPORT
    w_proc->http_srv = ymo_wsgi_server_init(
            w_proc->loop, w_proc->port, w_proc);

    if( !w_proc->http_srv ) {
        ymo_log_fatal("Failed to create HTTP server: %s", strerror(errno));
        return -1;
    }
#endif /* YMO_WSGI_REUSEPORT */
    return 0;
}


int get_module_app(ymo_wsgi_proc_t* w_proc, int argc, char** argv)
{
    size_t buf_len = 0;

    for( int i = 1; i < argc; i++ )
    {
        buf_len += strlen(argv[i]);
    }

    char* module = malloc(buf_len+1);

    if( !module ) {
        errno = ENOMEM;
        return -1;
    }

    module[0] = '\0';
    char* dst = module;
    for( int i = 1; i < argc; i++ )
    {
        size_t len = strlen(argv[i]);
        strcat(dst, argv[i]);
        dst += len;
    }

    char* app = strchr(module, ':');
    if( app == NULL ) {
        free(module);
        errno = EINVAL;
        return -1;
    }

    *app++ = '\0';
    w_proc->module = module;
    w_proc->app = app;

    if( strlen(w_proc->module) == 0 || strlen(w_proc->app) == 0 ) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

