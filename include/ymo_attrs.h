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



/** ymo_attrs.h
 * ==============
 *
 * Architecture and compiler features found at configure time.
 *
 * .. todo::
 *
 *    - Stop distributing config.h (ymo_config.h)
 *    - Create a tidy template file with the subset required in the dist
 *    - Make them overridable
 *
 * .. toctree::
 *   :hidden:
 *   :maxdepth: 3
 *
 * .. contents:: Contents
 *  :local:
 *  :depth: 3
 */

#ifndef YMO_ATTRS_H
#define YMO_ATTRS_H
#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <uuid/uuid.h>
#include <ev.h>

#include "ymo_config.h"

/*---------------------------------------------------------------*
 * Compiler Variable Attribute Wrappers:
 *---------------------------------------------------------------*/

#if defined(HAVE_VAR_ATTRIBUTE_MODE) && (HAVE_VAR_ATTRIBUTE_MODE == 1)
#define YMO_ATTR_MODE(x) __attribute__((mode(x)))
#define YMO_ENUM8_TYPEDEF(x) typedef enum x
#define YMO_ENUM8_AS(x) x YMO_ATTR_MODE(__byte__)
#define YMO_ENUM16_TYPEDEF(x) enum x
#define YMO_ENUM16_AS(x) ; typedef uint16_t x
// #define YMO_ENUM16_TYPEDEF(x) typedef enum x
// #define YMO_ENUM16_AS(x) x YMO_ATTR_MODE(HImode)
#else
#define YMO_ATTR_MODE(x)
#define YMO_ENUM8_TYPEDEF(x) enum x
#define YMO_ENUM8_AS(x) ; typedef uint8_t x
#define YMO_ENUM16_TYPEDEF(x) enum x
#define YMO_ENUM16_AS(x) ; typedef uint16_t x
#endif /* HAVE_VAR_ATTRIBUTE_MODE */

#if defined(HAVE_VAR_ATTRIBUTE_ALIGNED) && (HAVE_VAR_ATTRIBUTE_ALIGNED == 1)
#define YMO_ATTR_ALIGNED(x) __attribute__((aligned(x)))
#else
#define YMO_ATTR_ALIGNED(x)
#endif /* HAVE_VAR_ATTRIBUTE_ALIGNED */

/*---------------------------------------------------------------*
 * Compiler Function Attribute Wrappers:
 *---------------------------------------------------------------*/

#if defined(HAVE_FUNC_ATTRIBUTE_MALLOC) && (HAVE_FUNC_ATTRIBUTE_MALLOC == 1)
#  define YMO_FUNC_MALLOC __attribute__((malloc))
#  define YMO_FUNC_MALLOC_A malloc
#  define YMO_FUNC_MALLOC_P malloc,
#else
#  define YMO_FUNC_MALLOC
#  define YMO_FUNC_MALLOC_A
#  define YMO_FUNC_MALLOC_P
#endif /* HAVE_FUNC_ATTRIBUTE_MALLOC */

#if defined(HAVE_FUNC_ATTRIBUTE_UNUSED) && (HAVE_FUNC_ATTRIBUTE_UNUSED == 1)
#  define YMO_FUNC_UNUSED __attribute__((unused))
#  define YMO_FUNC_UNUSED_A unused
#  define YMO_FUNC_UNUSED_P unused,
#else
#  define YMO_FUNC_UNUSED
#  define YMO_FUNC_UNUSED_A
#  define YMO_FUNC_UNUSED_P
#endif /* HAVE_FUNC_ATTRIBUTE_UNUSED */

#if defined(HAVE_FUNC_ATTRIBUTE_FLATTEN) && (HAVE_FUNC_ATTRIBUTE_FLATTEN == 1)
#  define YMO_FUNC_FLATTEN __attribute__((flatten))
#  define YMO_FUNC_FLATTEN_A flatten
#  define YMO_FUNC_FLATTEN_P flatten,
#else
#  define YMO_FUNC_FLATTEN
#  define YMO_FUNC_FLATTEN_A
#  define YMO_FUNC_FLATTEN_P
#endif /* HAVE_FUNC_ATTRIBUTE_FLATTEN */

#if defined(HAVE_FUNC_ATTRIBUTE_FALLTHROUGH) && (HAVE_FUNC_ATTRIBUTE_FALLTHROUGH == 1)
#  define YMO_STMT_ATTR_FALLTHROUGH() __attribute__((fallthrough))
#else
#  define YMO_STMT_ATTR_FALLTHROUGH()
#endif /* HAVE_FUNC_ATTRIBUTE_FALLTHROUGH */

#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) && (HAVE_FUNC_ATTRIBUTE_WEAK == 1)
#  define YMO_FUNC_WEAK __attribute__((weak))
#  define YMO_FUNC_WEAK_A weak
#  define YMO_FUNC_WEAK_P weak,
#else
#  define YMO_FUNC_WEAK
#  define YMO_FUNC_WEAK_A
#  define YMO_FUNC_WEAK_P
#endif /* HAVE_FUNC_ATTRIBUTE_WEAK */

#if defined(HAVE_FUNC_ATTRIBUTE_WEAKREF) && (HAVE_FUNC_ATTRIBUTE_WEAKREF == 1)
#  define YMO_FUNC_WEAKREF(x) __attribute__((weakref(#x)))
#  define YMO_FUNC_WEAKREF_A(x) weakref(#x)
#  define YMO_FUNC_WEAKREF_P(x) weakref(#x),
#else
#  define YMO_FUNC_WEAKREF
#  define YMO_FUNC_WEAKREF_A
#  define YMO_FUNC_WEAKREF_P
#endif /* HAVE_FUNC_ATTRIBUTE_WEAKREF */

#if defined(HAVE_FUNC_ATTRIBUTE_ALIAS) && (HAVE_FUNC_ATTRIBUTE_ALIAS == 1)
#  define YMO_FUNC_ALIAS(x) __attribute__((alias(#x)))
#  define YMO_FUNC_ALIAS_A(x) alias(#x)
#  define YMO_FUNC_ALIAS_P(x) alias(#x),
#else
#  define YMO_FUNC_ALIAS
#  define YMO_FUNC_ALIAS_A
#  define YMO_FUNC_ALIAS_P
#endif /* HAVE_FUNC_ATTRIBUTE_ALIAS */

#endif /* YMO_ATTRS_H */




