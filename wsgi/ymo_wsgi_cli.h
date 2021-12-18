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



/** Command Line
 * ================
 * Utilities for the yimmo-wsgi command line.
 *
 *
 */

#ifndef YMO_WSGI_GREETING_H
#define YMO_WSGI_GREETING_H

#include "ymo_wsgi_proc.h"

/** TEMP: issue a greeting at yimmo-wsgi startup. */
void issue_startup_msg(ymo_wsgi_proc_t* w_proc);

#endif /* YMO_WSGI_GREETING_H */


