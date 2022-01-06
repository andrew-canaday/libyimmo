/*=============================================================================
 *
 *  Copyright (c) 2014 Andrew Canaday
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


#include "ymo_config.h"
#include <stdint.h>
#include <stdio.h>

#include "yimmo.h"
#include "ymo_util.h"
#include "ymo_alloc.h"

#define YMO_UTIL_TRACE 1
#if defined(YMO_UTIL_TRACE) && YMO_UTIL_TRACE == 1
# include "ymo_log.h"
# define UTIL_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
# define UTIL_TRACE_UUID(fmt, ...) ymo_log_trace_uuid(fmt, __VA_ARGS__);
#else
# define UTIL_TRACE(fmt, ...)
# define UTIL_TRACE_UUID(fmt, ...)
#endif /* YMO_UTIL_TRACE */


/*---------------------------------------------------------------*
 *  String case conversion macros:
 *---------------------------------------------------------------*/
/* Mask, high, and low values for max stride.
 * TODO: find size of uintmax_t and use that instead of stride_t
 *       See: https://github.com/andrew-canaday/fast_tolower/blob/2df8c96d4f7eff91226153bce3379a182e81b192/fast_tolower.h#L36-L40
 */
#if SIZEOF_SIZE_T == 8
/*----------------------------
*          64-bit:
*----------------------------*/
    #define  CASE_MASK_S  0x2020202020202020
    #define  L_LOW_S      0x4040404040404040
    #define  L_HIGH_S     0x5a5a5a5a5a5a5a5a
    #define  U_LOW_S      0x6060606060606060
    #define  U_HIGH_S     0x7a7a7a7a7a7a7a7a

typedef uint64_t stride_t;

#elif SIZEOF_SIZE_T == 4
/*----------------------------
*          32-bit:
*----------------------------*/
    #define  CASE_MASK_S  0x20202020
    #define  L_LOW_S      0x40404040
    #define  L_HIGH_S     0x5a5a5a5a
    #define  U_LOW_S      0x60606060
    #define  U_HIGH_S     0x7a7a7a7a

typedef uint32_t stride_t;

#elif SIZEOF_SIZE_T == 2
/*----------------------------
*          16-bit:
*----------------------------*/
    #define  CASE_MASK_S  0x20
    #define  L_LOW_S      0x40
    #define  L_HIGH_S     0x5a
    #define  U_LOW_S      0x60
    #define  U_HIGH_S     0x7a

typedef uint16_t stride_t;

#endif /* STRIDE_8 */

/*----------------------------
 *           8-bit:
 *----------------------------*/
#define  CASE_MASK  0x20
#define  L_LOW      0x40
#define  L_HIGH     0x5a
#define  U_LOW      0x60
#define  U_HIGH     0x7a


/*---------------------------------------------------------------*
 *  Convenience macros:
 *---------------------------------------------------------------*/
/* Convenience macro for converting a single char to lowercase: */
#define FAST_CHAR_TOLOWER(dst, src) \
    c = *src++; \
    mask = (((L_LOW - c) ^ (L_HIGH - c)) >> 2) & CASE_MASK; \
    *dst++ = c ^ mask;


/* Convenience macro for converting a single char to lowercase: */
#define FAST_CHAR_TOUPPER(dst, src) \
    c = *src++; \
    mask = (((U_LOW - c) ^ (U_HIGH - c)) >> 2) & CASE_MASK; \
    *dst++ = c ^ mask;


/*---------------------------------------------------------------*
 *  Utility Functions:
 *---------------------------------------------------------------*/
#if defined(YMO_TOLOWER_NO_INLINE) && YMO_TOLOWER_NO_INLINE
char ymo_tolower(char c)
{
    unsigned char m = (((0x40 - c) ^ (0x5a - c)) >> 2) & 0x20;
    return c ^ m;
}


char ymo_toupper(char c)
{
    unsigned char m = (((0x60 - c) ^ (0x7a - c)) >> 2) & 0x20;
    return c ^ m;
}


#endif /* YMO_TOLOWER_NO_INLINE */


void ymo_ntolower(char* dst, const char* src, size_t len)
{
    /* Number of iterations we can perform at maximum stride: */
    size_t no_iter = len / SIZEOF_SIZE_T;

    /* Number of single byte chunks to arrive at proper alignment: */
    size_t align = ((SIZEOF_SIZE_T) - \
                    ((stride_t)(src) % SIZEOF_SIZE_T)) % SIZEOF_SIZE_T;

    /* Number of single byte chunks after main no_iter is done: */
    size_t remain = (len % SIZEOF_SIZE_T) - align;

    const stride_t* src_s;
    stride_t* dst_s;
    size_t i;
    stride_t mask_s;
    stride_t c_s;
    uint8_t mask;
    char c;

    /* Iterate byte-by-byte until we achieve proper alignment for stride_t: */
    switch( align ) {
    #if SIZEOF_SIZE_T == 8
        case 7: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 6: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 5: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 4: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T == 8 */
    #if SIZEOF_SIZE_T >= 4
        case 3: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 2: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T >= 4 */
        case 1: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 0:
            YMO_STMT_ATTR_FALLTHROUGH();
        default:
            break;
    }

    /* Now iterate over the characters at the maximum stride possible: */
    src_s = (const stride_t*)src;
    dst_s = (stride_t*)dst;
    for( i = 0; i < no_iter; ++i )
    {
        c_s = *src_s++;
        mask_s = (((L_LOW_S - c_s) ^ (L_HIGH_S - c_s)) >> 2) & CASE_MASK_S;
        *dst_s++ = c_s ^ mask_s;
    }

    /* Convert remaining characters individually: */
    src = ((const char*)src_s);
    dst = ((char*)dst_s);
    switch( remain ) {
    #if SIZEOF_SIZE_T == 8
        case 7: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 6: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 5: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 4: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T == 8 */
    #if SIZEOF_SIZE_T >= 4
        case 3: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 2: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T >= 4 */
        case 1: FAST_CHAR_TOLOWER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 0:
            YMO_STMT_ATTR_FALLTHROUGH();
        default:
            break;
    }
}


void ymo_ntoupper(char* dst, const char* src, size_t len)
{
    /* Number of iterations we can perform at maximum stride: */
    size_t no_iter = len / SIZEOF_SIZE_T;

    /* Number of single byte chunks to arrive at proper alignment: */
    size_t align = ((SIZEOF_SIZE_T) - \
                    ((stride_t)(src) % SIZEOF_SIZE_T)) % SIZEOF_SIZE_T;

    /* Number of single byte chunks after main no_iter is done: */
    size_t remain = (len % SIZEOF_SIZE_T) - align;

    const stride_t* src_s;
    stride_t* dst_s;
    size_t i;
    stride_t mask_s;
    stride_t c_s;
    uint8_t mask;
    char c;

    /* Iterate byte-by-byte until we achieve proper alignment for stride_t: */
    switch( align ) {
    #if SIZEOF_SIZE_T == 8
        case 7: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 6: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 5: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 4: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T == 8 */
    #if SIZEOF_SIZE_T >= 4
        case 3: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 2: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T >= 4 */
        case 1: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 0:
            YMO_STMT_ATTR_FALLTHROUGH();
        default:
            break;
    }

    /* Now iterate over the characters at the maximum stride possible: */
    src_s = (const stride_t*)src;
    dst_s = (stride_t*)dst;
    for( i = 0; i < no_iter; ++i )
    {
        c_s = *src_s++;
        mask_s = (((U_LOW_S - c_s) ^ (U_HIGH_S - c_s)) >> 2) & CASE_MASK_S;
        *dst_s++ = c_s ^ mask_s;
    }

    /* Convert remaining characters individually: */
    src = ((const char*)src_s);
    dst = ((char*)dst_s);
    switch( remain ) {
    #if SIZEOF_SIZE_T == 8
        case 7: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 6: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 5: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 4: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T == 8 */
    #if SIZEOF_SIZE_T >= 4
        case 3: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 2: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
    #endif /* SIZEOF_SIZE_T >= 4 */
        case 1: FAST_CHAR_TOUPPER(dst, src)
            YMO_STMT_ATTR_FALLTHROUGH();
        case 0:
            YMO_STMT_ATTR_FALLTHROUGH();
        default:
            break;
    }
}


/*---------------------------------------------------------------*
 *  Base 64:
 *---------------------------------------------------------------*/
const char YMO_BASE64_TABLE[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+', '/',
};

#define YMO_B64_PAD '='


ymo_status_t ymo_base64_encode(
        char* dst, const unsigned char* src, size_t len)
{
    while( len > 2 ) {
        *dst++ = YMO_BASE64_TABLE[(src[0] >> 2) & 0x3f];
        *dst++ = YMO_BASE64_TABLE[((src[0] & 0x03) << 4) | (src[1] >> 4)];
        *dst++ = YMO_BASE64_TABLE[((src[1] & 0x0f) << 2) | (src[2] >> 6)];
        *dst++ = YMO_BASE64_TABLE[(src[2] & 0x03f)];
        src += 3;
        len -= 3;
    }

    if( len ) {
        *dst++ = YMO_BASE64_TABLE[(src[0] >> 2) & 0x3f];

        if( len == 1 ) {
            *dst++ = YMO_BASE64_TABLE[(src[0] & 0x03) << 4];
            *dst++ = '=';
        } else {
            *dst++ = YMO_BASE64_TABLE[((src[0] & 0x03) << 4) | (src[1] >> 4)];
            *dst++ = YMO_BASE64_TABLE[(src[1] & 0x0f) << 2];
        }

        *dst++ = '=';
    }
    *dst++ = '\0';
    return YMO_OKAY;
}


char* ymo_base64_encoded(const unsigned char* src, size_t len)
{
    char* dst = YMO_ALLOC( (4 * (len+2)) / 3);
    if( dst ) {
        ymo_base64_encode(dst, src, len);
    }
    return dst;
}


/*---------------------------------------------------------------*
 *  Base 64:
 *---------------------------------------------------------------*/
#define UTF8_WIDTH_1 0
#define UTF8_WIDTH_2 1
#define UTF8_WIDTH_3 2
#define UTF8_WIDTH_4 3

ymo_status_t ymo_check_utf8(
        ymo_utf8_state_t* state,
        const char*       buffer,
        size_t len,
        int done)
{
    static const uint8_t c3_min[4] = {
        0x00,
        0x80,
        0xA0,
        0xE0,
    };

    static const uint8_t c4_min[5] = {
        0x00,
        0x80,
        0x80,
        0x90,
        0xF0,
    };

    static const uint8_t c4_max[5] = {
        0x00,
        0xBF,
        0xBF,
        0x8F,
        0xF4,
    };

#define quartet(c,p) ( \
        (c << 24) | \
        (*(p) << 16) | \
        (*(p+1) << 8)  | \
        (*(p+2)) \
        )

    const char* p = buffer;
    while( len-- )
    {
        uint8_t c = (uint8_t)*p++;

        if( c > 0xF4 || c == 0xC0 || c == 0xC1 ) {
            UTIL_TRACE("Got invalid UTF-8 char: 0x%02x", (int)c);
            return EILSEQ;
        }

        /* TODO: TIDY: */
        if( state->point_remain == 0 ) {
            /* HACK: we can save some time by checking for strings of ascii.
             *
             * TODO:
             * - dubious whether checking 64 AND 32 is efficient...
             * - we can skip the shifts if we do this on alignment boundaries
             *   (but that also means we only check when a byte start happens
             *   ON an alignment boundary)
             * - we could do 128, 256, or 512 bits at a time with SIMD
             */
            if( len >= 4 && ((quartet(c,p) & 0x80808080) == 0) ) {
                len -= 3;
                p += 3;
                continue;
            }

            UTIL_TRACE("Got start byte: 0x%02x", (int)c);
            state->flags = 0;

            if( !(c & 0x80)) { /* Single byte: */
                UTIL_TRACE("Single byte: 0x%02x", (int)(int)c);
                state->point_remain = 0;
                state->code_width = UTF8_WIDTH_1;

            } else if( (c & 0xE0) == 0xC0 ) { /* Two bytes: */
                UTIL_TRACE("Two bytes: 0x%02x", (int)(int)c);
                state->point_remain = 1;
                state->code_width = UTF8_WIDTH_2;

            } else if( (c & 0xF0) == 0xE0 ) { /* Three bytes: */
                UTIL_TRACE("Three bytes: 0x%02x", (int)(int)c);
                state->point_remain = 2;
                state->code_width = UTF8_WIDTH_3;
                state->check_surrogate = (c == 0xED);
                state->check_overlong = (c == 0xE0);

            } else if( (c & 0xF8) == 0xF0 ) { /* Four bytes: */
                UTIL_TRACE("Four bytes: 0x%02x", (int)(int)c);
                state->point_remain = 3;
                state->code_width = UTF8_WIDTH_4;
                state->check_overlong = (c == 0xF0);
                state->check_max = (c == 0xF4);

            } else {
                UTIL_TRACE("Got invalid UTF-8 start byte: 0x%02x", (int)c);
                return EILSEQ;
            }

        } else if( (c & 0xC0) == 0x80 ) {
            UTIL_TRACE("Got continuation byte: 0x%02x", (int)(int)c);

            switch( state->code_width ) {

                /* TODO:
                 * Overlong checks: you only need to check the first.
                 */

                case UTF8_WIDTH_3:
                    if( state->check_overlong && c < c3_min[state->point_remain] ) {
                        UTIL_TRACE("Overlong encoding: 0x%02x", (int)c);
                        return EILSEQ;
                    } else {
                        state->check_overlong = 0;
                    }

                    if( state->check_surrogate && c >= 0xA0 ) {
                        UTIL_TRACE(
                                "Continuation yields char in range "
                                "U+D800 — U+DFFF: 0x%02x", (int)c);
                        return EILSEQ;
                    } else {
                        state->check_surrogate = 0;
                    }
                    break;

                case UTF8_WIDTH_4:
                    if( state->check_overlong && c < c4_min[state->point_remain] ) {
                        UTIL_TRACE("Overlong encoding: 0x%02x", (int)c);
                        return EILSEQ;
                    } else {
                        state->check_overlong = 0;
                    }

                    if( state->check_max && c > c4_max[state->point_remain] ) {
                        UTIL_TRACE("Invalid UTF-8 value too large: 0x%02x",
                                (int)c);
                        return EILSEQ;
                    }
                    break;
            }

            /* Got another byte for our code point */
            --state->point_remain;
        } else {
            /* Expected char continuation. */
            UTIL_TRACE("Expected UTF-8 continution byte. Got 0x%02x", (int)c);
            return EILSEQ;
        }
    }

    if( state->point_remain && done ) {
        UTIL_TRACE(
                "UTF-8 stream ended with truncated multi-byte. "
                "Expected %zu more", state->point_remain);
        return EILSEQ;
    }

    /* If we made it through the whole buffer, we're good so far. */
    return YMO_OKAY;
}

