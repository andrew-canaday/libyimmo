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

#define PY_SSIZE_T_CLEAN
#include "yimmo_config.h"

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>


#include "yimmo.h"
#include "ymo_alloc.h"
#include "ymo_log.h"

#include "ymo_http.h"

#include "ymo_wsgi_mod.h"
#include "ymo_wsgi_cli.h"
#include "ymo_wsgi_proc.h"

#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#endif

/** # ymo_wsgi_cli.c
 * Utilities for the yimmo-wsgi command line.
 *
 *
 */

/*---------------------------------*
 *            Globals:
 *---------------------------------*/
const char* GREETING_END =
    "\n"
    "----------------------------------------------------------------------------"
    "\n\n";

const char* GREETING_START =
    "\n\n"
    "*===========================================================================\n"
    "|               __     __\n"
    "|               \\ \\   / /\n"
    "|                \\ \\_/ / ( ) _ __ ___   _ __ ___    ___     \n"
    "|                 \\   /  | || '_ ` _ \\ | '_ ` _ \\  /   \\ [ ]\n"
    "|                  | |   | || | | | | || | | | | || ( ) |   \n"
    "|                  |_|   |_||_| |_| |_||_| |_| |_| \\___/ [ ]\n"
    "|\n"
    "*---------------------------------------------------------------------------\n";

/*---------------------------------*
 *         Type Hakcs:
 *---------------------------------*/
#define BACKEND_INFO(x) {x, #x}

typedef struct backend_info {
    int         id;
    const char* name;
} backend_info_t;


/*---------------------------------*
 *         Functions:
 *---------------------------------*/
void issue_startup_msg(ymo_wsgi_proc_t* w_proc)
{
    puts(GREETING_START);
    printf("Lib version: %i.%i.%i\n",
            YIMMO_VERSION_MAJOR,
            YIMMO_VERSION_MINOR,
            YIMMO_VERSION_PATCH);

    /* Log level info: */
    ymo_log_level_t level = ymo_log_get_level();

    puts("Logging:");
    printf("  - Log level (max): %s (%i)\n",
            ymo_log_get_level_name(YMO_LOG_LEVEL_MAX),
            YMO_LOG_LEVEL_MAX);
    printf("  - Log level (default): %s (%i)\n",
            ymo_log_get_level_name(YMO_LOG_LEVEL_DEFAULT),
            YMO_LOG_LEVEL_DEFAULT);
    printf("  - Log level (current): %s (%i)\n",
            ymo_log_get_level_name(level),
            level);
    printf("  - alloc override method: %i\n", YMO_ALLOC_LT_OVERRIDE);
    printf("  - header hash override method: %s\n",
            ymo_http_hdr_hash_override_method());

#ifdef YMO_WSGI_PRINT_EXTRA_INFO
    backend_info_t backends[] = {
        BACKEND_INFO(EVBACKEND_SELECT),
        BACKEND_INFO(EVBACKEND_POLL),
        BACKEND_INFO(EVBACKEND_EPOLL),
        BACKEND_INFO(EVBACKEND_KQUEUE),
        BACKEND_INFO(EVBACKEND_DEVPOLL),
        BACKEND_INFO(EVBACKEND_PORT),
        {0, NULL},
    };

    puts("stdatomic:");
    printf(" - ATOMIC_BOOL_LOCK_FREE:     %i\n", ATOMIC_BOOL_LOCK_FREE);
    printf(" - ATOMIC_CHAR_LOCK_FREE:     %i\n", ATOMIC_CHAR_LOCK_FREE);
    printf(" - ATOMIC_CHAR16_T_LOCK_FREE: %i\n", ATOMIC_CHAR16_T_LOCK_FREE);
    printf(" - ATOMIC_CHAR32_T_LOCK_FREE: %i\n", ATOMIC_CHAR32_T_LOCK_FREE);
    printf(" - ATOMIC_WCHAR_T_LOCK_FREE:  %i\n", ATOMIC_WCHAR_T_LOCK_FREE);
    printf(" - ATOMIC_SHORT_LOCK_FREE:    %i\n", ATOMIC_SHORT_LOCK_FREE);
    printf(" - ATOMIC_INT_LOCK_FREE:      %i\n", ATOMIC_INT_LOCK_FREE);
    printf(" - ATOMIC_LONG_LOCK_FREE:     %i\n", ATOMIC_LONG_LOCK_FREE);
    printf(" - ATOMIC_LLONG_LOCK_FREE:    %i\n", ATOMIC_LLONG_LOCK_FREE);
    printf(" - ATOMIC_POINTER_LOCK_FREE:  %i\n", ATOMIC_POINTER_LOCK_FREE);

    puts("EV Backends:");
    backend_info_t* backend = &backends[0];
    const char* supported;
    const char* recommended;
    while( backend->name != NULL ) {
        if( ev_supported_backends() & backend->id ) {
            supported = "Yes";
        } else {
            supported = "No";
        }

        if( ev_recommended_backends() & backend->id ) {
            recommended = "Yes";
        } else {
            recommended = "No";
        }

        printf(" - %s supported/recommended?: %s/%s\n",
                backend->name, supported, recommended);
        backend++;
    }
#endif /* YMO_WSGI_PRINT_EXTRA_INFO */

    /* Server Info: */
    printf("Http Port: %li\n", w_proc->port);
    printf("PID: %i\n", (int)getpid());
    puts(GREETING_END);
    return;
}

