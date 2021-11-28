/*=============================================================================
 *
 * Copyright (c) 2014 Andrew Canaday
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



/** Low-level Memory Management
 * =============================
 *
 *
 * Yimmo memory management API.
 *
 * This file defines a set of convenience macros used to wrap common memory
 * allocation/deallocation functions. Which functions are actually invoked are
 * dependent on configuration options.
 *
 * .. contents:: Contents
 *  :local:
 *  :depth: 3
 */

#ifndef YMO_ALLOC_H
#define YMO_ALLOC_H

#include <stdlib.h>

#include "yimmo.h"


/** Allocator Macros
 * ------------------
 */


/** .. c:macro:: YMO_ALLOC(n)
 *
 *  Allocates n bytes, aligned to at least sizeof(void*).
 *
 *  :param n: number of bytes to allocate.
 */

#ifndef YMO_ALLOC
#  define YMO_ALLOC(n)       ymo_malloc(n)
#endif /* YMO_ALLOC */

/** .. c:macro:: YMO_ALLOC0(n)
 *
 *  Allocates n bytes, zeroed and aligned to at least sizeof(void*).
 *
 *  :param n: number of bytes to allocate.
 */

#ifndef YMO_ALLOC0
#  define YMO_ALLOC0(n)      ymo_calloc(1, n)
#endif /* YMO_ALLOC0 */

/** .. c:macro:: YMO_FREE(p)
 *
 *  free n bytes of memory pointed to by p.
 *
 *  :param n: size of the block to free, as passed to :c:macro:`YMO_ALLOC`
 *  :param p: a void* to the memory being freed
 *
 *  **Example:**
 *
 *  ```C
 *  size_t n = ...;
 *  char* p = YMO_ALLOC(n);
 *  do_stuff(p);
 *  YMO_FREE(p);
 *  ```
 *
 */

#ifndef YMO_FREE
#  define YMO_FREE(p)        ymo_free(p)
#endif /* YMO_FREE */

/** .. c:macro:: YMO_NEW(t)
 *
 *  Allocate sufficient space to accommodate an object of type t.
 *
 *  :param t: a type name for which we need to allocate space
 *
 *  **Example:**
 *
 *  ```C
 *  // Allocates sizeof(my_type_t) bytes:
 *  my_type_t* my_obj = YMO_NEW(my_type_t);
 *  ```
 *
 */

#ifndef YMO_NEW
#  define YMO_NEW(t)         ymo_malloc(sizeof(t))
#endif /* YMO_NEW */

/** .. c:macro:: YMO_NEW0(t)
 *
 *  Allocate and zero sufficient space for an object of type t.
 *
 *  :param t: a type name for which we need to allocate space
 *
 *  **Example:**
 *
 *  ```C
 *  // Allocates sizeof(my_type_t) bytes:
 *  my_type_t* my_obj = YMO_NEW0(my_type_t);
 *  ```
 *
 */

#ifndef YMO_NEW0
#  define YMO_NEW0(t)        ymo_calloc(1, sizeof(t))
#endif /* YMO_NEW0 */

/** .. c:macro:: YMO_DELETE(t, p)
 *
 *  Deallocate memory allocated via :c:macro:`YMO_NEW` or :c:macro:`YMO_NEW0`.
 *
 *  :param t: the type that was allocated via :c:macro:`YMO_NEW` or :c:macro:`YMO_NEW0`
 *  :param p: a void* to the object being freed
 *
 *  **Example:**
 *
 *  ```C
 *  // Allocates sizeof(my_type_t) bytes:
 *  my_type_t* my_obj = YMO_NEW0(my_type_t);
 *  do_stuff(my_obj);
 *  YMO_DELETE(my_type_t, my_obj);
 *  ```
 *
 */


#ifndef YMO_DELETE
#  define YMO_DELETE(t, p)   ymo_free(p)
#endif /* YMO_DELETE */


/** Customization
 * ---------------
 *
 * The allocation macros above are all wrappers for one of the following three
 * functions: :c:func:`ymo_malloc`, :c:func:`ymo_calloc`, :c:func:`ymo_free`.
 *
 * (By default, these are either aliases or wrappers for ``malloc``, ``calloc``,
 * and ``free``).
 *
 * On platforms that support ``weakref``, ``weak, alias``, or ``weak``, these
 * are defined to be *weak* symbols. This allows the core memory routines to
 * be overridden by user-supplied routines by providing strong symbols at
 * link time.
 *
 * You can check to see if this mechanism is available using the
 * :c:macro:`YMO_ALLOC_LT_OVERRIDE` macro.
 *
 * **Example**:
 *
 * .. code-block:: c
 *
 *    #include "ymo_alloc.h"
 *
 *    #if YMO_ALLOC_LT_OVERRIDE
 *
 *    // Here's a silly ymo_malloc override which
 *    // just prints the number of bytes allocated:
 *    void* ymo_malloc(size_t n)
 *    {
 *        fprintf(stderr, "Allocating %zu bytes!\n", n);
 *        return malloc(n);
 *    }
 *    #endif
 *
 *
 * Default Allocator Functions
 * ...........................
 *
 * .. c:macro:: YMO_ALLOC_LT_OVERRIDE
 *
 *    Set to ``1`` if link-time memory customization is possible; ``0``, otherwise.
 *
 * .. c:function:: void* ymo_malloc(size_t n)
 *
 *    Wrapper/alias for ``malloc``.
 *
 * .. c:function:: void* ymo_calloc(size_t c, size_t n)
 *
 *    Wrapper/alias for ``calloc``.
 *
 * .. c:function:: void ymo_free(void* p)
 *
 *    Wrapper/alias for ``free``.
 *
 *
 * Disabling Link-Time Overrides
 * .............................
 *
 * The link-time override methods can be disabled at configure-time, even on
 * platforms which support them, by setting any of the following flags to ``0``:
 *
 * - ``YMO_ALLOC_ALLOW_WEAKREF``
 * - ``YMO_ALLOC_ALLOW_WEAK``
 * - ``YMO_ALLOC_ALLOW_ALIAS``
 *
 * To disable link-time overrides entirely, for example, you would do the
 * following:
 *
 * .. code-block:: bash
 *
 *    ../configure \
 *        YMO_ALLOC_ALLOW_WEAKREF=0 \
 *        YMO_ALLOC_ALLOW_WEAK=0 \
 *        YMO_ALLOC_ALLOW_ALIAS=0
 *
 *
 * Customizing at Compile-Time
 * ...........................
 *
 * On platforms which lack support for the linking mechanisms above, it is
 * possible to override these functions at compile-time by defining the
 * symbols via the preprocessor, e.g.:
 *
 * .. code-block:: bash
 *
 *    ../configure CFLAGS="-Dymo_malloc=my_custom_malloc"
 */

/* Check for weakref: */
#if defined(HAVE_FUNC_ATTRIBUTE_WEAKREF) \
    && (HAVE_FUNC_ATTRIBUTE_WEAKREF == 1) \
    && defined(YMO_ALLOC_ALLOW_WEAKREF) \
    && (YMO_ALLOC_ALLOW_WEAKREF == 1)

/*---------------------------------------------------------------------------*
 * WEAKREF:
 *--------------------------------------------------------------------------*/
#define YMO_ALLOC_LT_OVERRIDE 1

static void* ymo_malloc(size_t n)           __attribute__((YMO_FUNC_UNUSED_P weakref("malloc")));
static void* ymo_calloc(size_t c, size_t n) __attribute__((YMO_FUNC_UNUSED_P weakref("calloc")));
static void ymo_free(void* p)               __attribute__((YMO_FUNC_UNUSED_P weakref("free")));

/* Else: check for weak: */
#elif defined(HAVE_FUNC_ATTRIBUTE_WEAK) \
    && (HAVE_FUNC_ATTRIBUTE_WEAK == 1) \
    && defined(YMO_ALLOC_ALLOW_WEAK) \
    && (YMO_ALLOC_ALLOW_WEAK == 1)


/* Do we have alias too? */
#  if defined(HAVE_FUNC_ATTRIBUTE_ALIAS) \
    && (HAVE_FUNC_ATTRIBUTE_ALIAS == 1) \
    && defined (YMO_ALLOC_ALLOW_ALIAS) \
    && (YMO_ALLOC_ALLOW_ALIAS == 1)

/*---------------------------------------------------------------------------*
 * WEAK + ALIAS:
 *--------------------------------------------------------------------------*/
#define YMO_ALLOC_LT_OVERRIDE 1

void* ymo_malloc(size_t n)           __attribute__((weak, alias("malloc")));
void* ymo_calloc(size_t c, size_t n) __attribute__((weak, alias("calloc")));
void ymo_free(void* p)               __attribute__((weak, alias("free")));

/* If not, we'll just use weak: */
#  else

/*---------------------------------------------------------------------------*
 * WEAK ONLY:
 *--------------------------------------------------------------------------*/
#define YMO_ALLOC_LT_OVERRIDE 1

void* ymo_malloc(size_t n)           __attribute__((YMO_FUNC_MALLOC_P weak));
void* ymo_calloc(size_t c, size_t n) __attribute__((YMO_FUNC_MALLOC_P weak));
void ymo_free(void* p)               __attribute__((YMO_FUNC_MALLOC_P weak));

#  endif /* HAVE_FUNC_ATTRIBUTE_ALIAS */
#else /* !(HAVE_FUNC_ATTRIBUTE_WEAKREF) && (!HAVE_FUNC_ATTRIBUTE_WEAK) */
/*---------------------------------------------------------------------------*
 * NO WEAKREF, WEAK, or ALIAS:
 *
 * If we don't have any of the above, we'll just use preproc macros.
 * In this case, overrides have to happen at compile time.
 *
 *--------------------------------------------------------------------------*/
#define YMO_ALLOC_LT_OVERRIDE 0

#ifndef ymo_malloc
#  define ymo_malloc(n)    malloc(n)
#endif /* ymo_malloc */

#ifndef ymo_calloc
#  define ymo_calloc(c, n) calloc(c, n)
#endif /* ymo_calloc */

#ifndef ymo_free
#  define ymo_free(p)      free(p)
#endif /* ymo_free */

#endif /* HAVE_FUNC_ATTRIBUTE_WEAKREF */
#endif /* YMO_ALLOC_H */

