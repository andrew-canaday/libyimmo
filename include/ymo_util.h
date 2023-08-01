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

#include <stdlib.h>
#include <stdint.h>
#include "yimmo.h"

/** Utilities
 * =============
 */

/**---------------------------------------------------------------
 * Lookups:
 *---------------------------------------------------------------*/

/* Base 64 table: */
extern const unsigned char YMO_BASE64_TABLE[];

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

typedef union ymo_utf8_state {
    uint8_t  flags;
    struct {
        uint8_t  point_remain    : 2;
        uint8_t  code_width      : 2;
        uint8_t  check_surrogate : 1;
        uint8_t  check_overlong  : 1;
        uint8_t  check_max       : 1;
    };
} ymo_utf8_state_t;


/**---------------------------------------------------------------
 * Macros
 *---------------------------------------------------------------*/

/** Simple min macro.
 */
#define YMO_MIN(x,y) ((x < y) ? x : y)


/** Simple max macro.
 */
#define YMO_MAX(x,y) ((x > y) ? x : y)


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

#define YMO_PTR_ALIGN_MASK_TYPE \
        (void*)

#define YMO_PTR_ALIGN_MASK (sizeof(YMO_PTR_ALIGN_MASK_TYPE) - 1)

/** Round a pointer DOWN to the nearest void* alignment boundary.
 *
 */
#define YMO_PTR_FLOOR(p) (((uintptr_t)p) & ~YMO_PTR_ALIGN_MASK)

/** Round a pointer UP to the nearest void* alignment boundary.
 *
 */
#define YMO_PTR_CEIL(p) ((((uintptr_t)p) + YMO_PTR_ALIGN_MASK) & ~YMO_PTR_ALIGN_MASK)

/** .. c:macro:: YMO_TYPE_ALIGN(t)
 *
 * Get the alignment for the given type, ``t``.
 *
 */

#define YMO_UTIL_USE_ALIGNOF 0
#if defined(YMO_UTIL_USE_ALIGNOF) && YMO_UTIL_USE_ALIGNOF \
    && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define YMO_TYPE_ALIGN(t) _Alignof(t)
#else
#  define YMO_TYPE_ALIGN(t) (sizeof(void*))
#endif


#define YMO_PTR32_ALIGN_MASK (sizeof(uint32_t) - 1)

/** Round pointer value down to next 32-bit word boundary: */
#define YMO_PTR32_FLOOR(p) (((uintptr_t)p) & ~YMO_PTR32_ALIGN_MASK)

/** Round pointer value up to next 32-bit word boundary: */
#define YMO_PTR32_CEIL(p) ((((uintptr_t)p) + YMO_PTR32_ALIGN_MASK) & ~YMO_PTR32_ALIGN_MASK)

/** Round the input address DOWN to the nearest value that's a multiple
 * of the alignment for the type given by ``s``.
 */
#define YMO_TYPE_FLOOR(s, p) ( ((uintptr_t)p) & ~YMO_TYPE_ALIGN(s))

/** Round the input address UP to the nearest value that's a multiple
 * of the alignment for the type given by ``s``.
 */
#define YMO_TYPE_CEIL(s, p) (( ((uintptr_t)p) + YMO_TYPE_ALIGN(s)) & ~YMO_TYPE_ALIGN(s))

#define YMO_RESET_ERRNO() (errno = 0)



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
    YMO_FUNC_UNUSED YMO_FUNC_FLATTEN;

/** Fast, single-char, toupper function.
 *
 */
static inline char ymo_toupper(char c) \
    YMO_FUNC_UNUSED YMO_FUNC_FLATTEN;


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

/** Base64-encode a string into the given buffer.
 *
 * .. warning::
 *    ``dst`` must point to a location in memory with enough space
 *    to encode all of ``src`` as base64. Use :c:macro:`YMO_BASE64_LEN`
 *    to determine the length, if need be.
 *
 * :param dst: destination buffer
 * :param src: input string
 * :param len: length of ``src``
 * :returns: ``YMO_OKAY`` on success
 */
ymo_status_t ymo_base64_encode(char* dst, const char* src, size_t len);


/** Allocate enough space to store ``src`` base64-encoded, encode it
 * and return a pointer to the new buffer on success.
 *
 * :param src: input string
 * :param len: length of ``src``
 * :returns: pointer to new buffer on success; ``NULL`` on failure
 *
 * .. note::
 *    Use :c:macro:`YMO_FREE` to deallocate the returned buffer.
 *
 */
char* ymo_base64_encoded(const char* src, size_t len);


/** String compare when the length of both strings are known.
 *
 * :param s1: string 1
 * :param l1: string 1 length
 * :param s2: string 2
 * :param le: string 2 length
 * :returns: ``0`` if the strings are equal; **non-zero**, otherwise.
 */
static inline int ymo_strcmp(
        const char* s1, size_t l1,
        const char* s2, size_t l2
        )
{
    if( l1 == l2 ) {
        return memcmp(s1, s2, l1);
    }
    return -1;
}


/** Case insensitive string compare when the length of both strings are known.
 *
 * :param s1: string 1
 * :param l1: string 1 length
 * :param s2: string 2
 * :param le: string 2 length
 * :returns: ``0`` if the strings are equal; **non-zero**, otherwise.
 */
static inline int ymo_strcasecmp(
        const char* s1, size_t l1,
        const char* s2, size_t l2
        )
{
    if( l1 == l2++ ) {
        while( --l2 > 0 && ymo_tolower(*s1++) == ymo_tolower(*s2++) ) {}
        return l2;
    }
    return -1;
}


/** Initialize or reset a UTF-8 state object. */
#define ymo_utf8_state_reset(s) ((s)->flags = 0)

/** Check that a given string of bytes is valid UTF-8.
 *
 * :param state: present UTF-8 validator state
 * :param buffer: the buffer to check
 * :param len: the length of the buffer to check
 * :param done: if true, ``buffer + (len-1)`` is the last byte in the stream
 * :returns: ``YMO_OKAY`` on success; errno value otherwise
 *
 * .. code-block:: c
 *    :caption: Example
 *
 *    char buffer[BUFF_MAX];
 *    ymo_utf8_state_t state;
 *    ymo_utf8_state_reset(&state);
 *
 *    // (read some data into buffer)
 *
 *    // Check that what we've got so is valid:
 *    ymo_status_t u_status;
 *    if( (u_status = ymo_check_utf8(&state, buffer, len, 0)) != YMO_OKAY ) {
 *        ymo_log_info("Invalid UTF-8: %s", strerror(u_status));
 *    }
 *
 *    // (read some more data into buffer; set done=1 if that's the EOM)
 *    if( (u_status = ymo_check_utf8(&state, buffer, len, 1)) != YMO_OKAY ) {
 *        ymo_log_info("Invalid UTF-8: %s", strerror(u_status));
 *    }
 *
 */
ymo_status_t ymo_check_utf8(
        ymo_utf8_state_t* state,
        const char*       buffer,
        size_t len,
        int done);

#endif /* YMO_UTIL_H */



