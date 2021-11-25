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

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <stdlib.h>

#include "yimmo.h"
#include "ymo_config.h"
#include "ymo_log.h"

#include "ymo_wsgi_mod.h"
#include "ymo_wsgi_util.h"

/** # ymo_wsgi_util.c
 * Utilities for the yimmo-wsgi command line.
 *
 *
 */

int ymo_wsgi_signal_mask(int signum, int how)
{
    sigset_t signal_mask;

    if( how == SIG_SETMASK ) {
        ymo_log_error("Error. Attempt to SIG_SETMASK for %s in %i",
                strsignal(signum), (int)getpid());
        return EINVAL;
    }

    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, signum);

    errno = 0;
    int r_sig = pthread_sigmask(how, &signal_mask, NULL);
    if( r_sig ) {
        ymo_log_warning(
                "Failed to set signal mask for child process %i: %s",
                (int)getpid(), strerror(errno));
    } else {
        ymo_log_notice("Signal \"%s\" updated for %i to %s",
                strsignal(signum), (int)getpid(),
                how == SIG_BLOCK ? "BLOCK" : "UNBLOCK");
    }
    return r_sig;
}

int ymo_wsgi_signal_get_mask(sigset_t* signal_mask)
{
    return pthread_sigmask(0, NULL, signal_mask);
}


int ymo_wsgi_signal_set_mask(sigset_t* signal_mask)
{
    return pthread_sigmask(SIG_SETMASK, signal_mask, NULL);
}


