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



/** Utilities
 * =================
 * Miscellaneous utilities
 *
 *
 */

#ifndef YMO_WSGI_UTIL_H
#define YMO_WSGI_UTIL_H
#define PY_SSIZE_T_CLEAN
#include <Python.h>

/**---------------------------------
 *          Definitions
 *---------------------------------*/

/** */
#define YMO_INCREF_PYDICT_SETITEM(d, k, v) \
    { \
        PyObject* _v_py_tmp = v; \
        Py_INCREF(_v_py_tmp); \
        PyDict_SetItem(d, k, _v_py_tmp); \
    }

/** */
#define YMO_DECREF_PYDICT_SETITEM(d, k, v) \
    { \
        PyObject* _v_py_tmp = v; \
        PyDict_SetItem(d, k, _v_py_tmp); \
        Py_DECREF(_v_py_tmp); \
    }

/** */
#define YMO_INCREF_PYDICT_SETITEM_STRING(d, k, v) \
    { \
        PyObject* _v_py_tmp = v; \
        Py_INCREF(_v_py_tmp); \
        PyDict_SetItemString(d, k, _v_py_tmp); \
    }

/** */
#define YMO_DECREF_PYDICT_SETITEM_STRING(d, k, v) \
    { \
        PyObject* _v_py_tmp = v; \
        PyDict_SetItemString(d, k, _v_py_tmp); \
        Py_DECREF(_v_py_tmp); \
    }


/**---------------------------------
 *            Functions
 *---------------------------------*/

/** */
int ymo_wsgi_signal_mask(int signum, int how);

/** */
int ymo_wsgi_signal_get_mask(sigset_t* signal_mask);

/** */
int ymo_wsgi_signal_set_mask(sigset_t* signal_mask);


#endif /* YMO_WSGI_UTIL_H */


