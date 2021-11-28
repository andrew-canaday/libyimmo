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



/** Block Allocator
 * =================
 *
 */

#ifndef YMO_BLOCK_ALLOC_H
#define YMO_BLOCK_ALLOC_H
#include <stddef.h>
#include <stdalign.h>

#include "yimmo.h"


/**---------------------------------------------------------------
 * Macros
 *---------------------------------------------------------------*/

#define YMO_TYPE_ALIGN(s) (s - 1)

/** Round the input address DOWN to the nearest value that's a multiple
 * of the alignment for the type given by ``s``.
 */
#define YMO_TYPE_FLOOR(s, p) ( ((uintptr_t)p) & ~YMO_TYPE_ALIGN(s))

/** Round the input address UP to the nearest value that's a multiple
 * of the alignment for the type given by ``s``.
 */
#define YMO_TYPE_CEIL(s, p) (( ((uintptr_t)p) + YMO_TYPE_ALIGN(s)) & ~YMO_TYPE_ALIGN(s))

/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/* HACK: if FLEXIBLE_ARRAY_MEMBER isn't defined, assume C99 and set it
 *       to nothing. If need be, the user can define it as 1 for older
 *       compilers:
 */
#ifndef FLEXIBLE_ARRAY_MEMBER
#define FLEXIBLE_ARRAY_MEMBER
#endif /* FLEXIBLE_ARRAY_MEMBER */

typedef unsigned char blit_t;

/** Structure used to wrap stack-style block storage.
 */
typedef struct ymo_blalloc {
    blit_t* current;
    blit_t* end;
    size_t  total;
    size_t  remain;
    blit_t  val[FLEXIBLE_ARRAY_MEMBER];
} ymo_blalloc_t;


/**---------------------------------------------------------------
 * Functions
 *---------------------------------------------------------------*/

/** Create a block alloc pool.
 *
 * :param n: size, in bytes, of the pool to create.
 * :returns: pointer to :c:type:`ymo_blalloc_t` on success; ``NULL``, otherwise.
 */
ymo_blalloc_t* ymo_blalloc_create(size_t n) YMO_FUNC_MALLOC;

/** Allocate a type from the block
 * e.g. my_type_t* my_var = YMO_BLALLOC(block, my_type_t)
 */
#define YMO_BLALLOC(b,t) \
    ymo_blalloc(b, alignof(t), sizeof(t))

/** Allocate a type from the block (prefer the YMO_BLALLOC(b, t) macro)
 * e.g. ymo_blalloc(block, alignof(my_type_t), sizeof(my_type_t))
 * e.g. ymo_blalloc(block, alignof(char), strlen(my_str))
 *
 * :param t_align: type alignment (per alignof)
 * :param t_size: type size in bytes (per sizeof)
 * :returns: allocated space on success; ``NULL`` with ``errno`` set on failure.
 */
void* ymo_blalloc(ymo_blalloc_t* block, size_t a, size_t n);

/** strdup implemented on top of blalloc.
 * If there's space enough: you'll get a copy.
 * Otherwise, it'll return NULL.
 *
 * :param block: memory block used to allocate storage for s
 * :param s: the string to copy
 * :returns: a pointer to the duplicated string on success; ``NULL`` with ``errno`` set, otherwise
 */
char* ymo_blalloc_strdup(struct ymo_blalloc* block, const char* s);

/** Reset a block alloc pool.
 *
 * :param block: the pool to reset
 */
void ymo_blalloc_reset(ymo_blalloc_t* block);

/** Free a block alloc pool.
 *
 * :param block: the pool to free
 */
void ymo_blalloc_free(ymo_blalloc_t* block);

#endif /* YMO_BLOCK_ALLOC_H */


