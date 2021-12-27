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

#include <stdlib.h>
#include <string.h>

#include "ymo_alloc.h"
#include "ymo_blalloc.h"

ymo_blalloc_t* ymo_blalloc_create(size_t n)
{
    size_t remain = n * sizeof(blit_t);
    ymo_blalloc_t* block = (ymo_blalloc_t*)YMO_ALLOC(
            offsetof(ymo_blalloc_t, val) + remain);

    if( block == NULL ) {
        return block;
    }

    block->remain = remain;
    block->current = block->val;
    block->end = block->current + remain;
    return block;
}


void* ymo_blalloc(ymo_blalloc_t* block, size_t t_align, size_t t_size)
{
    blit_t* obj_start = (blit_t*)YMO_TYPE_CEIL(t_align, block->current);
    blit_t* obj_end = obj_start + t_size;

    if( obj_end < block->end ) {
        block->remain -= (obj_end - block->current);
        block->current = obj_end;
        return (void*)obj_start;
    }

    return NULL;
}


char* ymo_blalloc_strdup(ymo_blalloc_t* block, const char* s)
{
    size_t len = strlen(s);
    char* buffer = (char*)ymo_blalloc(block, alignof(char), len+1);
    if( buffer ) {
        strcpy(buffer, s);
    }
    return buffer;
}


void ymo_blalloc_reset(ymo_blalloc_t* block)
{
    block->current = block->val;
    block->remain = block->end - block->val;
}


void ymo_blalloc_free(ymo_blalloc_t* block)
{
    if( block != NULL ) {
        YMO_DELETE(ymo_blalloc_t, block);
    }
}

