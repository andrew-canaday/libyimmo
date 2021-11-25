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

#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "ymo_config.h"
#include "ymo_trace_alloc.h"

struct ymo_alloc_record {
    struct ymo_alloc_record* next;
    const char*              filename;
    const char*              funcname;
    const char*              alloctype;
    size_t                   line;
    size_t                   no_alloc;
    size_t                   no_dealloc;
    unsigned char            mem[FLEXIBLE_ARRAY_MEMBER];  /* TODO: align! */
};

static ymo_alloc_record_t* trace_alloc_records = NULL;

void* ymo_trace_alloc(
        const char* filename,
        const char* funcname,
        size_t line,
        const char* alloctype,
        size_t len,
        int zero
        )
{
    ymo_alloc_record_t* record = malloc(
            sizeof(ymo_alloc_record_t) + len);
    if( !record ) {
        goto trace_alloc_bail;
    }

    record->filename = filename;
    record->funcname = funcname;
    record->line = line;
    record->alloctype = alloctype;
    record->no_alloc = len;

    if( zero ) {
        memset(record->mem, 0, len);
    }

    record->next = trace_alloc_records;
    trace_alloc_records = record;
    return record;

trace_alloc_bail:
    free(record);
    return NULL;
}

void ymo_trace_dealloc(
        const char* filename,
        const char* funcname,
        size_t line,
        const char* alloctype,
        void* p
        )
{
    ymo_alloc_record_t* record = p - offsetof(ymo_alloc_record_t, mem);
    record->no_dealloc = record->no_alloc;
    return;
}

void ymo_memcheck(void)
{
    ymo_alloc_record_t* record = trace_alloc_records;

    while( record ) {
        if( record->no_alloc != record->no_dealloc ) {
            fprintf(stderr, "LEAK! %s:%s:%zu (%s)\n",
                    record->filename,
                    record->funcname,
                    record->line,
                    record->alloctype
                    );
        } else {
            fprintf(stderr, "OK: %s:%s:%zu (%s)\n",
                    record->filename,
                    record->funcname,
                    record->line,
                    record->alloctype
                    );
        }
        record = record->next;
    }
    return;
}

