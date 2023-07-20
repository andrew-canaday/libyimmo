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


#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_ws.h"

#include "yimmo_wsgi.h"
#include "ymo_wsgi_util.h"
#include "ymo_py_websockets.h"


/*----------------------------------------------------------------------------
 *
 * Definitions:
 *
 *----------------------------------------------------------------------------*/
#define ymo_wsgi_ws_argcheck(arg) \
    if( arg != NULL && !PyCallable_Check(arg) ) { \
        PyErr_SetString(PyExc_TypeError, \
        "Parameter :\"" #arg "\" must be callable or None."); \
        return NULL; \
    }


/*----------------------------------------------------------------------------
 *
 * Types:
 *
 *----------------------------------------------------------------------------*/
struct yimmo_websocket {
    PyObject_HEAD
    ymo_ws_session_t* session;
};

typedef struct ymo_wsgi_ws_callbacks {
    PyObject* on_open;
    PyObject* on_message;
    PyObject* on_close;
} ymo_wsgi_ws_callbacks_t;


static ymo_wsgi_ws_callbacks_t ws_callbacks = {
    .on_open = NULL,
    .on_message = NULL,
    .on_close = NULL
};


/*----------------------------------------------------------------------------
 *
 * yimmo.WebSocket:
 *
 *----------------------------------------------------------------------------*/
static PyObject* yimmo_WebSocket_send(
        yimmo_websocket_t* self, PyObject* const* args, Py_ssize_t nargs)
{
    char* py_data;
    Py_ssize_t py_len;

    if( nargs != 2 ) {
        PyErr_SetString(PyExc_TypeError,
                "yimmo.WebSocket.send expects at two"
                " arguments (msg and flags).");
        return NULL;
    }

    if( !PyBytes_Check(args[0]) ) {
        PyErr_SetString(PyExc_TypeError,
                "yimmo.WebSocket.send msg parameter must be a bytes object.");
        return NULL;
    }

    if( !PyLong_Check(args[1]) ) {
        PyErr_SetString(PyExc_TypeError,
                "yimmo.WebSocket.send flags parameter must be an int.");
        return NULL;
    }

    PyBytes_AsStringAndSize(args[0], &py_data, &py_len);
    long flags = PyLong_AsLong(args[1]);
    ymo_bucket_t* msg_out = YMO_BUCKET_FROM_CPY(py_data, py_len);
    ymo_ws_session_send(self->session, flags, msg_out);
    Py_RETURN_NONE;
}


static PyObject* yimmo_WebSocket_close(
        yimmo_context_t* self, PyObject* const* args, Py_ssize_t nargs)
{
    /* No-op */
    Py_RETURN_NONE;
}


PyMethodDef yimmo_WebSocketType_methods[] = {
    {"send", (PyCFunction) yimmo_WebSocket_send, METH_FASTCALL,
     "Send a websocket message"},
    {"close", (PyCFunction) yimmo_WebSocket_close, METH_FASTCALL,
     "Close the connection"},
    {NULL}  /* Sentinel */
};


PyTypeObject yimmo_WebSocketType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "yimmo.WebSocket",
    .tp_doc = "YIMMO WebSocket",
    .tp_alloc = PyType_GenericAlloc,
    .tp_new = PyType_GenericNew,
    .tp_basicsize = sizeof(yimmo_websocket_t),
    .tp_itemsize = 0,
    .tp_methods = yimmo_WebSocketType_methods,
    .tp_flags = Py_TPFLAGS_DEFAULT,
};


/*----------------------------------------------------------------------------
 *
 * yimmo.init_websockets:
 *
 *----------------------------------------------------------------------------*/
PyObject* yimmo_init_websockets(
        PyObject* self, PyObject* args, PyObject* kwargs)
{
    static char* kwlist[] = {
        "on_open",
        "on_message",
        "on_close",
        NULL
    };

    PyObject* on_open = NULL;
    PyObject* on_message = NULL;
    PyObject* on_close = NULL;

    if( !PyArg_ParseTupleAndKeywords(
            args, kwargs, "OOO", kwlist,
            &on_open, &on_message, &on_close)) {
        return NULL;
    }

    ymo_wsgi_ws_argcheck(on_open);
    ymo_wsgi_ws_argcheck(on_message);
    ymo_wsgi_ws_argcheck(on_close);

    ws_callbacks.on_open = on_open;
    ws_callbacks.on_message = on_message;
    ws_callbacks.on_close = on_close;
    Py_RETURN_NONE;
}


/*----------------------------------------------------------------------------
 *
 * libyimmo_ws callbacks:
 *
 *----------------------------------------------------------------------------*/
ymo_status_t ymo_wsgi_ws_connect_cb(ymo_ws_session_t* session)
{
    ymo_status_t r_status = YMO_OKAY;

    PyGILState_STATE gstate = PyGILState_Ensure();
    yimmo_websocket_t* ws = YIMMO_WEBSOCKET_C(
            yimmo_WebSocketType.tp_alloc(&yimmo_WebSocketType, 0));
    if( ws ) {
        Py_INCREF(ws);
        ws->session = session;
        ymo_ws_session_set_userdata(session, ws);
    } else {
        r_status = ENOMEM;
    }

    if( !ws || !ws_callbacks.on_open ) {
        PyGILState_Release(gstate);
        return r_status;
    }

    PyObject* pArgs = PyTuple_New(1);
    Py_INCREF(YIMMO_WEBSOCKET_PY(ws));
    PyTuple_SetItem(pArgs, 0, YIMMO_WEBSOCKET_PY(ws));
    PyObject* r_val = PyObject_CallObject(
            ws_callbacks.on_open, pArgs);

    if( !r_val ) {
        r_status = EBADMSG;
    } else if( r_val != Py_None ) {
        if( PyLong_Check(r_val) ) {
            r_status = PyLong_AsLong(r_val);
        } else {
            r_status = EINVAL;
        }
    }

    Py_XDECREF(r_val);
    PyGILState_Release(gstate);
    return r_status;
}


ymo_status_t ymo_wsgi_ws_recv_cb(
        ymo_ws_session_t* session,
        void*             user_data,
        uint8_t flags,
        const char*       data,
        size_t len)
{
    ymo_status_t r_status = YMO_OKAY;

    if( !ws_callbacks.on_message ) {
        return EINVAL;
    }

    /* HACK HACK: for now, we'll just subject libyimmo to the GIL to get
     *            WS operational-ish.
     */
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* pArgs = PyTuple_New(3);
    Py_INCREF(YIMMO_WEBSOCKET_PY(user_data));
    PyTuple_SetItem(pArgs, 0, YIMMO_WEBSOCKET_PY(user_data));
    PyTuple_SetItem(pArgs, 1, PyBytes_FromStringAndSize(data, len));
    PyTuple_SetItem(pArgs, 2, PyLong_FromLong(flags));
    PyObject* r_val = PyObject_CallObject(
            ws_callbacks.on_message, pArgs);

    if( !r_val ) {
        r_status = EBADMSG;
    } else if( r_val != Py_None ) {
        if( PyLong_Check(r_val) ) {
            r_status = PyLong_AsLong(r_val);
        } else {
            r_status = EINVAL;
        }
    }

    Py_XDECREF(r_val);
    PyGILState_Release(gstate);
    return r_status;
}


void ymo_wsgi_ws_close_cb(ymo_ws_session_t* session, void* user_data)
{
    if( !ws_callbacks.on_close ) {
        return;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* pArgs = PyTuple_New(1);
    Py_INCREF(YIMMO_WEBSOCKET_PY(user_data));
    PyTuple_SetItem(pArgs, 0, YIMMO_WEBSOCKET_PY(user_data));
    PyObject* r_val = PyObject_CallObject(
            ws_callbacks.on_close, pArgs);

    Py_XDECREF(r_val);
    PyGILState_Release(gstate);
    return;
}

