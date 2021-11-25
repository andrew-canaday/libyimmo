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

#include <stdlib.h>

/** Trace Allocator
 * =================
 *
 * .. warning::
 *    To work properly, this header *must be included before any other
 *    yimmo headers!*
 *
 * Provide tracing overrides for the allocators in ymo_alloc.h.
 */

#ifndef YMO_TRACE_ALLOC_H
#define YMO_TRACE_ALLOC_H

typedef struct ymo_alloc_record ymo_alloc_record_t;

void* ymo_trace_alloc(
        const char* filename,
        const char* funcname,
        size_t line,
        const char* alloctype,
        size_t len,
        int zero
        );

void ymo_trace_dealloc(
        const char* filename,
        const char* funcname,
        size_t line,
        const char* alloctype,
        void* p
        );


void ymo_memcheck(void);


#define YMO_ALLOC(n) \
    ymo_trace_alloc(__FILE__, __func__, __LINE__, "(no type)", n, 0)

#define YMO_ALLOC0(n) \
    ymo_trace_alloc(__FILE__, __func__, __LINE__, "(no type)", n, 1)

#define YMO_FREE(t, p) \
    ymo_trace_dealloc(__FILE__, __func__, __LINE__, #t, p)

#define YMO_NEW(t) \
    ymo_trace_alloc(__FILE__, __func__, __LINE__, #t, sizeof(t), 0)

#define YMO_NEW0(t) \
    ymo_trace_alloc(__FILE__, __func__, __LINE__, #t, sizeof(t), 1)

#define YMO_DELETE(t, p) \
    ymo_trace_dealloc(__FILE__, __func__, __LINE__, #t, p)

#endif /* YMO_TRACE_ALLOC_H */
