/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 * Copyright (c) 2014 Andrew Canaday
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



#ifndef YMO_UTIL_H
#define YMO_UTIL_H

#include "ymo_config.h"
#include <stdlib.h>
#include <stdint.h>
#include "yimmo.h"

/** Utilities
 * =============
 */

/*---------------------------------------------------------------*
 * Tables
 *---------------------------------------------------------------*/

/* Base 64 table: */
extern const char YMO_BASE64_TABLE[];


/**---------------------------------------------------------------
 * Macros
 *---------------------------------------------------------------*/

/** Simple min macro.
 */
#define YMO_MIN(x,y) ((x < y) ? x : y)


/** Fast single-char to lower macro.
 */
#define YMO_TOLOWER(c) \
    (c ^ (( ((0x40 - c) ^ (0x5a - c)) >> 2) & 0x20))

/** Fast single-char to upper macro.
 */
#define YMO_TOUPPER(c) \
    (c ^ (( ((0x60 - c) ^ (0x7a - c)) >> 2) & 0x20))

/** Given a string length in bytes, return the number of bytes required to
 * store the same string, base64 encoded.
 *
 */
#define YMO_BASE64_LEN(len) \
    ( (4*(len+2)) / 3)

#define YMO_PTR_ALIGN (sizeof(void*) - 1)

/** Round a pointer DOWN to the nearest alignment boundary.
 *
 */
#define YMO_PTR_FLOOR(p) (((uintptr_t)p) & ~YMO_PTR_ALIGN)

/** Round a pointer UP to the nearest alignment boundary.
 *
 */
#define YMO_PTR_CEIL(p) ((((uintptr_t)p) + YMO_PTR_ALIGN) & ~YMO_PTR_ALIGN)


/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

#if defined(YMO_TOLOWER_NO_INLINE) && (YMO_TOLOWER_NO_INLINE == 1)
char ymo_tolower(char c);
#else /* defined(YMO_TOLOWER_NO_INLINE) && YMO_TOLOWER_NO_INLINE */

/** Fast, single-char, tolower function.
 *
 */
static inline char ymo_tolower(char c) \
    YMO_FUNC_ATTR_UNUSED YMO_FUNC_ATTR_FLATTEN;

/** Fast, single-char, toupper function.
 *
 */
static inline char ymo_toupper(char c) \
    YMO_FUNC_ATTR_UNUSED YMO_FUNC_ATTR_FLATTEN;

static inline char ymo_tolower(char c)
{
    unsigned char m = (((0x40 - c) ^ (0x5a - c)) >> 2) & 0x20;
    return c ^ m;
}

static char ymo_toupper(char c)
{
    unsigned char m = (((0x60 - c) ^ (0x7a - c)) >> 2) & 0x20;
    return c ^ m;
}
#endif /* YMO_INLINE_TOLOWER */

/** Fast full-string tolower implementation.
 *
 * (See `fast_tolower <https://github.com/andrew-canaday/fast_tolower>`_).
 *
 * :param dst: destination string (may be same as src)
 * :param src: source string (may be same as dst)
 * :param len: number of characters in the string
 *
 */
void ymo_ntolower(char* dst, const char* src, size_t len);

/** Fast full-string toupper implementation.
 *
 * (See `fast_tolower <https://github.com/andrew-canaday/fast_tolower>`_).
 *
 * :param dst: destination string (may be same as src)
 * :param src: source string (may be same as dst)
 * :param len: number of characters in the string
 *
 */
void ymo_ntoupper(char* dst, const char* src, size_t len);

ymo_status_t
ymo_base64_encode(char* dst, const unsigned char* src, size_t len);

char*
ymo_base64_encoded(const unsigned char* src, size_t len);

#endif /* YMO_UTIL_H */



