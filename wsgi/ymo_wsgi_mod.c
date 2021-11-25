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

/** # ymo_wsgi_mod.c
 * Setup for the yimmo python module.
 *
 *
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "ymo_log.h"

#include "yimmo_wsgi.h"
#include "ymo_wsgi_mod.h"
#include "ymo_wsgi_context.h"


/*---------------------------------*
 *             Globals:
 *---------------------------------*/
/* Only used in this translation unit: */
static PyObject* pWsgiAppModule = NULL;

/* Module-level object dependencies: */
PyObject* pSys = NULL;
PyObject* pWsgiAppCallable = NULL;

/* Environ Values (set once — we can use SetItemString for these): */
PyObject* pStderr = NULL;
PyObject* pWsgiVersion = NULL;
PyObject* pUrlScheme = NULL;
PyObject* pCommonEnviron = NULL;

/* Environ Keys (reused — save time using SetItem): */
PyObject* pEnvironKeyInput = NULL;
PyObject* pEnvironKeyRequestMethod = NULL;
PyObject* pEnvironKeyPathInfo = NULL;
PyObject* pEnvironKeyQueryString = NULL;
PyObject* pEnvironKeyScriptName = NULL;
PyObject* pEnvironKeyContentType = NULL;
PyObject* pEnvironKeyContentLength = NULL;

/* Common attribute names (resued — save time using GetAttr): */
PyObject* pAttrWrite = NULL;
PyObject* pAttrClose = NULL;
PyObject* pAttrStartResponse = NULL;


/*---------------------------------*
 *          Prototypes:
 *---------------------------------*/
PyMODINIT_FUNC ymo_wsgi_module_init();

/*---------------------------------*
 *           Functions:
 *---------------------------------*/

ymo_status_t ymo_wsgi_init(const char* mod_name, const char* app_name)
{
    /*------------------------------------------------------------------
     * Startup Python and Initialize the yimmo module:
     *------------------------------------------------------------------*/
    ymo_log_notice("Starting embedded python interpretter");
    Py_InitializeEx(0);

    ymo_log_notice("Notifying python of process fork");
#if PY_VERSION_HEX <= 0x03070000
    PyOS_AfterFork();
#else
    PyOS_AfterFork_Child();
#endif /* PY_VERSION_HEX */
    PyObject* m = ymo_wsgi_module_init();

    if( !m ) {
        fprintf(stderr, "Unable to initialize ymo_wsgi python module!\n");
        return -1;
    }

    /*------------------------------------------------------------------
     * Get the WSGI application or bail:
     *------------------------------------------------------------------*/
    PyObject* pWsgiAppName = PyUnicode_DecodeFSDefault(mod_name);
    if( !pWsgiAppName ) {
        goto module_bail;
    }

    pWsgiAppModule = PyImport_Import(pWsgiAppName);
    Py_XDECREF(pWsgiAppName);

    if( !pWsgiAppModule ) {
        goto module_bail;
    }

    pWsgiAppCallable = PyObject_GetAttrString(pWsgiAppModule, app_name);
    if( !pWsgiAppCallable ) {
        goto module_bail;
    }

    if( !PyCallable_Check(pWsgiAppCallable) ) {
        ymo_log_error("WSGI app argument (%s) must be callable!", app_name);
        goto module_bail;
    }

    /*------------------------------------------------------------------
     * There are a lot of objects reused from request to request
     * without modification. Bootstrap them here and just Py_INCREF 'em.
     *------------------------------------------------------------------*/
    pSys = PyImport_Import(PyUnicode_FromString("sys"));
    if( !pSys ) {
        goto module_bail;
    }

    pStderr = PyObject_GetAttrString(pSys, "stderr");
    if( !pStderr ) {
        goto module_bail;
    }

    pWsgiVersion = PyTuple_New(2);
    if( !pWsgiVersion ) {
        goto module_bail;
    }

    PyTuple_SetItem(pWsgiVersion, 0, PyLong_FromLong(1));
    PyTuple_SetItem(pWsgiVersion, 1, PyLong_FromLong(0));

    /* TODO: properly set URL scheme */
    pUrlScheme = PyUnicode_FromString("http");
    if( !pUrlScheme ) {
        goto module_bail;
    }

    /* Common Keys: */

    /* Environment parameters that don't change from one request to the next:
     * (TODO: error checking for all *_New and _*From* calls!)
     */
    pCommonEnviron = PyDict_New();

    /* PEP 3333: */
    /* TODO: "SERVER_NAME" / "SERVER_PORT" */
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "SERVER_PROTOCOL", pUrlScheme);
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "SCRIPT_NAME", PyUnicode_FromString("yimmo-wsgi"));

    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "wsgi.version", pWsgiVersion);
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "wsgi.errors", pStderr); /* HACK! */
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "wsgi.url_scheme", pUrlScheme);
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "wsgi.multithreaded", Py_True);
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "wsgi.multiprocess", Py_False);
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "wsgi.run_once", Py_False);

    /* Werkzeug: */
    YMO_INCREF_PYDICT_SETITEM_STRING(pCommonEnviron, "wsgi.input_terminated", Py_True);

    /* Environ Keys (reused — save time using SetItem): */
    pEnvironKeyInput = PyUnicode_InternFromString("wsgi.input");
    pEnvironKeyRequestMethod = PyUnicode_InternFromString("REQUEST_METHOD");
    pEnvironKeyPathInfo = PyUnicode_InternFromString("PATH_INFO");
    pEnvironKeyQueryString = PyUnicode_InternFromString("QUERY_STRING");
    pEnvironKeyContentType = PyUnicode_InternFromString("CONTENT_TYPE");
    pEnvironKeyContentLength = PyUnicode_InternFromString("CONTENT_LENGTH");

    /* Common attribute names (resued — save time using GetAttr): */
    pAttrWrite = PyUnicode_InternFromString("write");
    pAttrClose = PyUnicode_InternFromString("close");
    pAttrStartResponse = PyUnicode_InternFromString("start_response");

    return YMO_OKAY;

module_bail:
    ymo_wsgi_shutdown();
    return ENOMEM;
}

int ymo_wsgi_shutdown()
{
    /* Don't bother tearing down, if we never stood up! */
    if( !Py_IsInitialized() ) {
        return 1;
    }

    /* Only used in this translation unit: */
    Py_XDECREF(pWsgiAppModule);

    /* Module-level object dependencies: */
    Py_XDECREF(pSys);
    Py_XDECREF(pWsgiAppCallable);

    /* Environ Values (set once — we can use SetItemString for these): */
    Py_XDECREF(pStderr);
    Py_XDECREF(pWsgiVersion);
    Py_XDECREF(pUrlScheme);
    Py_XDECREF(pCommonEnviron);

    /* Environ Keys (reused — save time using SetItem): */
    Py_XDECREF(pEnvironKeyInput);

    /* Common attribute names (resued — save time using GetAttr): */
    Py_XDECREF(pAttrWrite);
    Py_XDECREF(pAttrClose);
    Py_XDECREF(pAttrStartResponse);

    /* If there was some exception: print it out (lazy...) */
    if( PyErr_Occurred() ) {
        PyErr_Print();
    }

    /* If there was some error shutting down, be truthful: */
    if( Py_FinalizeEx() < 0 ) {
        return 120;
    }
    return 0;
}

/*---------------------------------------------------------------------------*
 *                              Module Init:
 *---------------------------------------------------------------------------*/
static PyModuleDef module_Yimmo = {
    PyModuleDef_HEAD_INIT,
    .m_name = "yimmo",
    .m_doc = "YIMMO WSGI Server.",
    .m_size = -1,
};


PyMODINIT_FUNC
ymo_wsgi_module_init()
{
    PyObject* m;
    if( PyType_Ready(&yimmo_ContextType) < 0 )
        return NULL;

    m = PyModule_Create(&module_Yimmo);
    if( m == NULL )
        return NULL;

    Py_INCREF(&yimmo_ContextType);
    if( PyModule_AddObject(
            m, "Request", (PyObject*)&yimmo_ContextType) < 0 ) {
        Py_DECREF(&yimmo_ContextType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}




