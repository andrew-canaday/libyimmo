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
 */

#ifndef YMO_ALLOC_H
#define YMO_ALLOC_H

#include "ymo_config.h"
#include "yimmo.h"

/** .. c:macro:: YMO_ALLOC(n)
 *
 *  Allocates n bytes, aligned to at least sizeof(void*).
 *
 *  :param n: number of bytes to allocate.
 */

/** .. c:macro:: YMO_ALLOC0(n)
 *
 *  Allocates n bytes, zeroed and aligned to at least sizeof(void*).
 *
 *  :param n: number of bytes to allocate.
 */

/** .. c:macro:: YMO_FREE(n, p)
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
 *  YMO_FREE(n, p);
 *  ```
 *
 */

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


#include <stdlib.h>

#ifndef YMO_ALLOC_ALIAS
#  define YMO_ALLOC_ALIAS    0
#endif /* YMO_ALLOC_ALIAS */

#ifndef YMO_ALLOC_WEAK
/* Off by default (VERY COSTLY) */
#  define YMO_ALLOC_WEAK     0
#endif /* YMO_ALLOC_WEAK */

#ifndef YMO_ALLOC_WEAKREF
/* WHOOPS: this is blorked. Will fix. */
#  define YMO_ALLOC_WEAKREF  0
#endif /* YMO_ALLOC_WEAKREF */

#ifndef YMO_ALLOC_PREPROC0
#  define YMO_ALLOC_PREPROC0 1
#endif /* YMO_ALLOC_PREPROC0 */



#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) \
    && (HAVE_FUNC_ATTRIBUTE_WEAK == 1)

/*---------------------------------------------------------------------------*
 * WEAK + ALIAS:
 *
 * - functions are global, weak, aliases to stdlib functions
 * - user can override without recompiling libyimmo
 * - platform semantics dictate which function is loaded at runtime
 *--------------------------------------------------------------------------*/
#  if defined(HAVE_FUNC_ATTRIBUTE_ALIAS) \
    && (HAVE_FUNC_ATTRIBUTE_ALIAS == 1) \
    && defined (YMO_ALLOC_ALIAS) \
    && (YMO_ALLOC_ALIAS == 1)

void* ymo_alloc(size_t n)  __attribute__((weak, alias("malloc")));
void* ymo_alloc0(size_t n) __attribute__((weak, alias("calloc")));
void ymo_free(void* p)    __attribute__((weak, alias("free")));

#    define YMO_ALLOC(n)       ymo_alloc(n)
#    define YMO_ALLOC0(n)      ymo_alloc0(1, n)
#    define YMO_FREE(t, p)     ymo_free(p)
#    define YMO_NEW(t)         ymo_alloc(sizeof(t))
#    define YMO_NEW0(t)        ymo_alloc0(1, sizeof(t))
#    define YMO_DELETE(t, p)   ymo_free(p)


/*---------------------------------------------------------------------------*
 * WEAK:
 *
 * - functions are global, weak, symbols in libyimmo shared lib
 * - user can override without recompiling libyimmo
 * - platform semantics dictate which function is loaded at runtime
 *--------------------------------------------------------------------------*/
#  elif defined(YMO_ALLOC_WEAK) \
    && (YMO_ALLOC_WEAK == 1)

void* ymo_alloc(size_t n);
void* ymo_alloc0(size_t n);
void ymo_free(void* p);

#    define YMO_ALLOC(n)       ymo_alloc(n)
#    define YMO_ALLOC0(n)      ymo_alloc0(n)
#    define YMO_FREE(t, p)     ymo_free(p)
#    define YMO_NEW(t)         ymo_alloc(sizeof(t))
#    define YMO_NEW0(t)        ymo_alloc0(sizeof(t))
#    define YMO_DELETE(t, p)   ymo_free(p)

/*---------------------------------------------------------------------------*
 * WEAKREF:
 *
 * - functions are static, inlined, weak, symbols in each translation unit
 * - user can override without recompiling libyimmo
 * - platform semantics dictate which function is loaded at runtime
 *--------------------------------------------------------------------------*/
#  elif defined(YMO_ALLOC_WEAKREF) \
    && (YMO_ALLOC_WEAKREF == 1)

YMO_FUNC_ATTR_UNUSED static void* ymo_alloc(size_t n)            __attribute__((weakref("_ymo_alloc")));
YMO_FUNC_ATTR_UNUSED static void* ymo_alloc0(size_t c, size_t n) __attribute__((weakref("_ymo_alloc0")));
YMO_FUNC_ATTR_UNUSED static void ymo_free(void* p)              __attribute__((weakref("_ymo_free")));

#    define YMO_ALLOC(n)       ymo_alloc(n)
#    define YMO_ALLOC0(n)      ymo_alloc0(1, n)
#    define YMO_FREE(t, p)     ymo_free(p)
#    define YMO_NEW(t)         ymo_alloc(sizeof(t))
#    define YMO_NEW0(t)        ymo_alloc0(1, sizeof(t))
#    define YMO_DELETE(t, p)   ymo_free(p)

#  endif /* HAVE_FUNC_ATTRIBUTE_ALIAS */
#endif /* HAVE_FUNC_ATTRIBUTE_WEAK */

/*-------------------------------------------
 * NO ALIAS/WEAK:
 *
 * TODO: Allow swap at compilation time or
 *      use function pointers.
 *
 *------------------------------------------*/
#ifndef YMO_ALLOC

#  if defined(YMO_ALLOC_PREPROC0) && (YMO_ALLOC_PREPROC0 == 1)
#    define ymo_alloc0(n) calloc(1, n)
#    else

YMO_FUNC_ATTR_UNUSED static inline void* ymo_alloc0(size_t n)
{
    char* buff = malloc(n);
    if( buff ) {
        memset(buff, 0, n);
    }
    return buff;
}

#    endif

#  define YMO_ALLOC(n)      malloc(n)
#  define YMO_ALLOC0(n)     ymo_alloc0(n)
#  define YMO_FREE(t, p)    free(p)
#  define YMO_NEW(t)        malloc(sizeof(t))
#  define YMO_NEW0(t)       ymo_alloc0(sizeof(t))
#  define YMO_DELETE(t, p)  free(p)

#endif /* YMO_ALLOC */

#endif /* YMO_ALLOC_H */

