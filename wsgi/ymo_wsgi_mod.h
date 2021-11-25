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




/** yimmo Module
 * ================
 * Setup for the yimmo python module.
 *
 *
 */

#ifndef YMO_WSGI_MOD_H
#define YMO_WSGI_MOD_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "yimmo.h"

#include "ymo_wsgi_util.h"

/**---------------------------------
 *          Definitions
 *---------------------------------*/

/** Default port to listen on for http connections. */
#define DEFAULT_HTTP_PORT 8081

/**---------------------------------
 *            Globals
 *---------------------------------*/

/** Module-level object dependencies: */
extern PyObject* pSys;
extern PyObject* pWsgiAppCallable;

/** Environ Values (set once — we can use SetItemString for these): */
extern PyObject* pStderr;
extern PyObject* pWsgiVersion;
extern PyObject* pUrlScheme;
extern PyObject* pCommonEnviron;

/** Environ Keys (reused — save time using SetItem): */
extern PyObject* pEnvironKeyInput;
extern PyObject* pEnvironKeyRequestMethod;
extern PyObject* pEnvironKeyPathInfo;
extern PyObject* pEnvironKeyQueryString;
extern PyObject* pEnvironKeyContentType;
extern PyObject* pEnvironKeyContentLength;

/** Common attribute names (resued — save time using GetAttr): */
extern PyObject* pAttrWrite;
extern PyObject* pAttrClose;
extern PyObject* pAttrStartResponse;

/**---------------------------------
 *         Functions
 *---------------------------------*/

/** Main WSGI initialization routine:
 *
 * - creates the HTTP server
 * - spawns initial processes
 * - multi proc mode: start watching child processes
 * - single proc mode: start python interpretter
 */
ymo_status_t ymo_wsgi_init(const char* mod_name, const char* app_name);

/** Stop the WSGI server.
 */
int ymo_wsgi_shutdown(void);

/** .. py:module:: yimmo */

#endif /* YMO_WSGI_MOD_H */


