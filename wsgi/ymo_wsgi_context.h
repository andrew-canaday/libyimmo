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




#ifndef YMO_WSGI_REQ_CTX_H
#define YMO_WSGI_REQ_CTX_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "yimmo_wsgi.h"


/** yimmo.Context
 * ====================
 * Contains the WSGI PEP3333 interface seen by python
 *
 *
 */

/** .. note::
 *    Yimmo WSGI function naming uses the following pattern:
 *
 *     - python object methods: ``yimmo_Context_<method>``
 *     - C-only functions to manipulate python objects: ``ymo_wsgi_ctx_<fn>``
 */

extern PyTypeObject yimmo_ContextType;
extern PyMethodDef yimmo_ContextType_methods[];

/**---------------------------------------------------------------------------
 *                        C Utility Functions
 *---------------------------------------------------------------------------*/

/** Update ``environ`` with values from ``exchange``:
 */
yimmo_context_t* ymo_wsgi_ctx_update_environ(
        PyObject* pEnviron, ymo_wsgi_exchange_t* exchange);

/** Return the exchange for this context.
 */
ymo_wsgi_exchange_t* ymo_wsgi_ctx_exchange(yimmo_context_t* ctx);


/**---------------------------------------------------------------------------
 *                           yimmo.Context
 *---------------------------------------------------------------------------*/

/** .. py:currentmodule:: yimmo
 *
 * .. py:class:: Context
 *
 * This class encapsulates the exchange information for a given session, as
 * well as providing the input/output streams and other facilities required
 * by `PEP3333`_.
 *
 */

/** .. py:method:: Context.__del__() */
void yimmo_Context_dealloc(yimmo_context_t* self);

/** .. py:method:: Context.start_response(status, response_headers, exc_info=None)
 *
 * PEP 3333 start_response
 *
 */
PyObject* yimmo_Context_start_response(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs);


/*----------------------- wsgi.input: --------------------------
 * PEP 3333: the following MUST be supported by "wsgi.input":
 *
 * - ``read(size)``
 * - ``readline()``
 * - ``readlines(hint)``
 * - ``__iter__()``
 *--------------------------------------------------------------*/

/** .. py:method:: Context.read(size=-1)
 *
 * Implements the PEP3333 read(size) method for wsgi.input.
 * The semantics are the same as io.RawIOBase.read(size=-1).
 *
 * The size parameter must be passed as a positional argument, not a kkwarg.
 */
PyObject* yimmo_Context_read(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs);


/** .. py:method:: Context.readline(size=-1)
 *
 * Implements the PEP3333 readline(size) method for wsgi.input.  The semantics
 * are the same as io.IOBase.readline(size=-1).  The size parameter must be
 * passed as a positional argument, not a kwarg.
 *
 */
PyObject* yimmo_Context_readline(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs);


/** .. py:method:: Context.readlines(hint=-1)
 *
 * Implements the PEP3333 readlines(hint=-1) method for wsgi.input. The
 * semantics are almost the same as io.IOBase.readlines(size=-1), save for the
 * hint exception allowed by PEP3333: the hint parameter is ignored. The hint
 * parameter must be passed as a positional argument, not a kkwarg.
 *
 */
PyObject* yimmo_Context_readlines(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs);


/** .. py:method:: Context.__iter__()
 *
 * Support the iterator protocol for WSGI context request bodies.
 */
PyObject* yimmo_Context_iter(yimmo_context_t* self);
PyObject* yimmo_Context_iternext(yimmo_context_t* self);


/** .. py:method:: Context.write(b)
 *
 * PEP 3333: imperative write callback for content data (backwards
 * compatibility for applications that don't provide data as a return value).
 *
 */
PyObject* yimmo_Context_write(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs);


/*------------------------ wsgi.errors: -------------------------*
 * PEP 3333: the following MUST be supported by "wsgi.errors":
 *
 * - ``flush()``
 * - ``write(str``)
 * - ``writelines(seq)``
 *
 * HACK: we just use sys.stderr for "wsgi.errors"
 *---------------------------------------------------------------*/
#endif /* YMO_WSGI_REQ_CTX_H */



