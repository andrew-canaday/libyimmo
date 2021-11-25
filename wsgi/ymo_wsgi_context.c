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

/** NOTE: Don't complain if we're not fastidious about field values when
 *       initializing struct data!
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "yimmo.h"
#include "ymo_util.h"
#include "ymo_log.h"
#include "ymo_http.h"

#include "ymo_wsgi_mod.h"
#include "ymo_wsgi_server.h"
#include "ymo_wsgi_session.h"
#include "ymo_wsgi_context.h"
#include "ymo_wsgi_worker.h"

/* TODO: HACK HACK HACK: per-header NAME buffer size limit. */
#define HDR_NAME_BUFFER_SIZE 256

/** # ymo_wsgi_context.c
 * Contains the WSGI PEP3333 interface seen by python
 *
 *
 */

/*---------------------------------*
 *            Types:
 *---------------------------------*/
struct yimmo_context {
    PyObject_HEAD
    ymo_wsgi_session_t*  session;
    ymo_wsgi_exchange_t* exchange;
};

/* TEMPORARY HACK: */
#define BODY_PTR self->exchange->request->body
#define BODY_LEN self->exchange->request->content_length

/*---------------------------------------------------------------------------*
 *                              yimmo.Context Methods:
 *---------------------------------------------------------------------------*/
void yimmo_Context_dealloc(yimmo_context_t* self)
{
    YMO_WSGI_TRACE("Deleting yimmo.Context: %p", (void*)self);
    ymo_wsgi_exchange_decref(self->exchange);
    Py_TYPE(self)->tp_free(YIMMO_CONTEXT_PY(self));
}


PyObject* yimmo_Context_start_response(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs)
{
    /* If exc_info is non-null, assume it's a sys.exc_info() type-tuple and
     * reraise by simply returning NULL.
     */
    if( nargs > 2 && args[2] != Py_None ) {
        /* Per PEP 3333:
         * - if any headers have already been sent: re-raise
         * - otherwise, process the new headers and return as usual
         *
         * NOTE: it's a lot to track what the other thread is doing, so....
         *       we just (for the moment) tell the app that headers have
         *       been sent by re-raising...
         */
        /* TODO: do we need to do anything here, or is it sufficient to
         * return NULL?
         * PyObject* exc_info = args[2];
         */
        return NULL;
    }

    /* If we have insufficient args: raise TypeError */
    if( nargs < 2 ) {
        PyErr_SetString(PyExc_TypeError,
                "yimmo.Context.start_response expects at least two"
                " arguments (environ and start_response).");
        return NULL;
    }

    /* Trylock fails if session is closed or libyimmo is using it — i.e. it is
     * "closed" or "closing".
     */
    if( ymo_wsgi_session_trylock(self->session) ) {
        PyErr_SetString(PyExc_ConnectionResetError,
                "Attempted to start_response on closed connection.");
        return NULL;
    }

    /* At this point, we're returning *something*, no matter what.
     * Per PEP 3333, it should be the write callable.
     */
    PyObject* pWrite =
        PyObject_GetAttr(YIMMO_CONTEXT_PY(self), pAttrWrite);

    /* Set the status string.
     * TODO: check type/validity!
     */
    PyObject* py_status = PyUnicode_AsASCIIString(args[0]);
    char* status_str = PyBytes_AsString(py_status);
    ymo_http_response_set_status_str(self->exchange->response, status_str);
    Py_DECREF(py_status);

    /* Copy the headers between python and C: */
    PyObject* headers = args[1];
    Py_ssize_t no_headers = PyList_Size(headers);

    for( Py_ssize_t i = 0; i < no_headers; i++ ) {
        /* TODO should we be checking types/validity here? */
        PyObject* hdr = PyList_GetItem(headers, i);
        PyObject* py_name = PyUnicode_AsASCIIString(PyTuple_GetItem(hdr, 0));
        PyObject* py_val = PyUnicode_AsASCIIString(PyTuple_GetItem(hdr, 1));

        char* hdr_name = ymo_blalloc_strdup(
                self->exchange->request->ws, PyBytes_AsString(py_name));
        char* hdr_val = ymo_blalloc_strdup(
                self->exchange->request->ws, PyBytes_AsString(py_val));

        Py_DECREF(py_name);
        Py_DECREF(py_val);
        if( hdr_name && hdr_val ) {
            ymo_http_response_set_header(
                    self->exchange->response, hdr_name, hdr_val);
        }
    }
    ymo_wsgi_session_unlock(self->session);
    return pWrite;
}


/* Context.read(size=-1) */
PyObject* yimmo_Context_read(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs)
{
    /* TODO: Raise BlockingIOError when no input available? */
    YMO_WSGI_TRACE("Got read on %p with nargs %lu",
            (void*)self, (unsigned long)nargs);

    size_t body_read = self->exchange->body_read;
    size_t content_length = BODY_LEN;
    if( !content_length || body_read >= content_length ) {
        YMO_WSGI_TRACE("Done (read %zu bytes)", body_read);
        return PyBytes_FromString("");
    }

    size_t body_remain = content_length - body_read;
    size_t chunk_length = body_remain;
    if( nargs ) {
        chunk_length = YMO_MIN(PyLong_AsSize_t(args[0]), body_remain);
        YMO_WSGI_TRACE("%p.read() returning %zu bytes",
                (void*)self, chunk_length);
    }

    const char* chunk_start = BODY_PTR + body_read;
    body_read += chunk_length;
    self->exchange->body_read = body_read;
    YMO_WSGI_TRACE("Read %zu bytes", body_read);
    return PyBytes_FromStringAndSize(chunk_start, chunk_length);
}


/* Context.readline(size=-1) */
PyObject* yimmo_Context_readline(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs)
{
    YMO_WSGI_TRACE("Got readline on %p with nargs %lu",
            (void*)self, (unsigned long)nargs);

    size_t body_read = self->exchange->body_read;
    size_t content_length = BODY_LEN;
    if( !content_length || body_read >= content_length ) {
        return PyBytes_FromString("");
    }

    size_t body_remain = content_length - body_read;
    size_t chunk_length = body_remain;
    if( nargs ) {
        chunk_length = YMO_MIN(PyLong_AsSize_t(args[0]), body_remain);
        YMO_WSGI_TRACE("%p.readline() returning %zu bytes",
                (void*)self, chunk_length);
    }

    const char* chunk_start = BODY_PTR + body_read;

    /* Look for a newline within the bounds of chunk_start, returning only
     * a single line, if possible.
     */
    for( size_t i = 0; i < chunk_length; i++ ) {
        if( chunk_start[i] == '\n' ) {
            chunk_length = i+1;
            break;
        }
    }

    body_read += chunk_length;
    self->exchange->body_read = body_read;
    PyObject* pLine = PyBytes_FromStringAndSize(chunk_start, chunk_length);
    if( !pLine ) {
        PyErr_SetString(PyExc_MemoryError,
                "Failed to allocate memory for body data");
    }
    return pLine;
}


/* Context.readline(hint=-1) */
PyObject* yimmo_Context_readlines(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs)
{
#if defined(YIMMO_WSGI_READLINES_HINT) && (YIMMO_WSGI_READLINES_HINT == 1)
    long hint = -1;
    if( nargs ) {
        PyObject* pHint = args[0];
        if( PyLong_Check(pHint) ) {
            hint = PyLong_AsLong(pHint);
        }
    }
#endif /* YIMMO_WSGI_READLINES_HINT */

    PyObject* pLine = NULL;
    PyObject* pList = PyList_New(0);

    /* On success, start populating the list.
     * If NULL, the interpretter has set the exception, so we just return. */
    if( pList ) {
        int have_lines = 1;
        while( have_lines ) {
            PyObject* pLine = yimmo_Context_readline(self, NULL, 0);
            if( pLine ) {
                if( PyBytes_GET_SIZE(pLine) ) {
                    int r_append = PyList_Append(pList, pLine);
                    if( r_append == -1 ) {
                        goto readlines_exc_bail;
                    }
                } else {
                    /* Empty line == we're done */
                    have_lines = 0;
                }
                Py_DECREF(pLine);
            } else {
                goto readlines_exc_bail;
            }
        }
    }

    return pList;

readlines_exc_bail:
    /* In all of the error conditions above, someone else has already set
     * the exception, so we just return NULL. */
    Py_XDECREF(pLine);
    Py_DECREF(pList);
    return NULL;
}


/* Context.__iter__
 *
 * NOTE: at the moment, this is easy — we just make the request its own
 * iterator and disallow multiple traversals.
 */
PyObject* yimmo_Context_iter(yimmo_context_t* self)
{
    Py_INCREF(YIMMO_CONTEXT_PY(self));
    return YIMMO_CONTEXT_PY(self);
}


PyObject* yimmo_Context_iternext(yimmo_context_t* self)
{
    PyObject* pData = NULL;

    size_t body_read = self->exchange->body_read;
    size_t content_length = BODY_LEN;

    if( body_read < content_length ) {
        size_t remain = content_length - body_read;
        char* start = BODY_PTR + body_read;
        YMO_WSGI_TRACE("Issuing %zu bytes", remain);
        pData = PyBytes_FromStringAndSize(start, remain);

        if( pData ) {
            self->exchange->body_read += remain;
        }
    } else {
        YMO_WSGI_TRACE("Stop iteration (%zu bytes read)", content_length);
        PyErr_SetNone(PyExc_StopIteration);
    }

    return pData;
}


/* Context.write(b)
 */
PyObject* yimmo_Context_write(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs)
{
    PyObject* body_data = args[0];
    ymo_bucket_t* body = NULL;
    Py_ssize_t content_length = 0;
    if( PyBytes_Check(body_data) ) {
        content_length = PyBytes_GET_SIZE(body_data);
        char* content = PyBytes_AS_STRING(body_data);
        body = YMO_BUCKET_FROM_CPY(content, content_length);
        ymo_wsgi_worker_response_body_append(self->exchange, body, 1);
    }
    PyLong_FromSize_t(content_length);
}


/*---------------------------------------------------------------------------*
 *                           Type Definition:
 *---------------------------------------------------------------------------*/
PyMethodDef yimmo_ContextType_methods[] = {
    {"start_response", (PyCFunction) yimmo_Context_start_response, METH_FASTCALL,
     "Start the WSGI response"},
    {"read", (PyCFunction) yimmo_Context_read, METH_FASTCALL,
     "I/O stream read method for body content"},
    {"readline", (PyCFunction) yimmo_Context_readline, METH_FASTCALL,
     "I/O stream readline method for body content"},
    {"readlines", (PyCFunction) yimmo_Context_readlines, METH_FASTCALL,
     "I/O stream readlines method for body content"},
    {"write", (PyCFunction) yimmo_Context_write, METH_FASTCALL,
     "I/O stream write method for http response"},
    {NULL}  /* Sentinel */
};


PyTypeObject yimmo_ContextType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "yimmo.Context",
    .tp_doc = "YIMMO WSGI yimmo_Context",
    .tp_basicsize = sizeof(yimmo_context_t),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)yimmo_Context_dealloc,
    .tp_methods = yimmo_ContextType_methods,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_iter = (getiterfunc)yimmo_Context_iter,
    .tp_iternext = (iternextfunc)yimmo_Context_iternext
};


/*---------------------------------------------------------------------------*
 *                       C-Only Object Creation:
 *---------------------------------------------------------------------------*/
yimmo_context_t* ymo_wsgi_ctx_update_environ(PyObject* pEnviron, ymo_wsgi_exchange_t* exchange)
{
    yimmo_context_t* ctx = NULL;

    ymo_wsgi_exchange_incref(exchange);
    if( ymo_wsgi_session_trylock(exchange->session) ) {
        /* Release the ref held for us: */
        ymo_wsgi_exchange_decref(exchange);
        errno = EINVAL;
        return NULL;
    }

    /* If we can't allocate the python type, just bail: */
    ctx = YIMMO_CONTEXT_C(
            yimmo_ContextType.tp_alloc(&yimmo_ContextType, 0));
    if( !ctx ) {
        /* Release the ref held for us: */
        ymo_wsgi_exchange_decref(exchange);
        errno = ENOMEM;
        goto ymo_wsgi_ctx_update_bail;
    }

    YMO_WSGI_TRACE("Created yimmo.Context: %p", (void*)ctx);
    /* Okay, we're cool. */
    ctx->session = exchange->session;
    ctx->exchange = exchange;

    PyDict_SetItem(pEnviron, pEnvironKeyInput, YIMMO_CONTEXT_PY(ctx));

    char hdr_field[HDR_NAME_BUFFER_SIZE] = "HTTP_";
    char* e_hdr_name = &hdr_field[5];

    /* ---- end HACK ---- */

    const char* hdr_name;
    const char* hdr_value;
    size_t key_len;

    ymo_http_request_t* request = exchange->request;
    ymo_http_hdr_ptr_t iter = ymo_http_hdr_table_next(
            &request->headers, NULL, &hdr_name, &key_len, &hdr_value);
    while( iter )
    {
        for( size_t i = 0; i <= key_len; i++ )
        {
            char c = hdr_name[i];
            if( c != '-' ) {
                if( c != '_' ) {
                    e_hdr_name[i] = ymo_toupper(c);
                } else {
                    /* NOTE: To prevent header spoofing during normalization,
                     * we ignore any headers with an underscore in them:
                     */
                    goto hdr_skip_underscore;
                }
            } else {
                e_hdr_name[i] = '_';
            }
        }
        YMO_DECREF_PYDICT_SETITEM_STRING(pEnviron, hdr_field,
                PyUnicode_FromString(hdr_value));

hdr_skip_underscore:
        iter = ymo_http_hdr_table_next(
                &request->headers, iter, &hdr_name, &key_len, &hdr_value);
    }


    /* Update environ:
     * TODO: error checking for all *_New and _*From* calls!
     */
    YMO_DECREF_PYDICT_SETITEM(pEnviron, pEnvironKeyRequestMethod,
            PyUnicode_FromString(request->method));
    YMO_DECREF_PYDICT_SETITEM(pEnviron, pEnvironKeyPathInfo,
            PyUnicode_FromString(request->uri));
    if( request->query ) {
        YMO_DECREF_PYDICT_SETITEM(pEnviron, pEnvironKeyQueryString,
                PyUnicode_FromString(request->query));
    }

    const char* content_type = ymo_http_hdr_table_get(&request->headers,
            "content-type");

    if( content_type ) {
        YMO_DECREF_PYDICT_SETITEM(pEnviron, pEnvironKeyContentType,
                PyUnicode_FromString(content_type));
    }
    if( request->content_length ) {
        YMO_DECREF_PYDICT_SETITEM(pEnviron, pEnvironKeyContentLength,
                PyLong_FromSize_t(request->content_length));
    }
ymo_wsgi_ctx_update_bail:
    ymo_wsgi_session_unlock(exchange->session);
    return ctx;
}


ymo_wsgi_exchange_t* ymo_wsgi_ctx_exchange(yimmo_context_t* ctx)
{
    return ctx->exchange;
}

#pragma GCC diagnostic pop



