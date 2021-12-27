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
#include "ymo_alloc.h"

#if defined(YMO_ALLOC_WEAK) \
    && (YMO_ALLOC_WEAK == 1) \
    && defined(HAVE_FUNC_ATTRIBUTE_WEAK) \
    && (HAVE_FUNC_ATTRIBUTE_WEAK == 1)

void* ymo_malloc(size_t n) __attribute__((YMO_FUNC_MALLOC_P weak))
{
    return malloc(n);
}


void* ymo_calloc(size_t c, size_t n) __attribute__((YMO_FUNC_MALLOC_P weak))
{
    return calloc(c,n);
}


void ymo_free(void* p) __attribute__((YMO_FUNC_MALLOC_P weak))
{
    free(p);
}


#endif /* YMO_ALLOC_WEAK */

