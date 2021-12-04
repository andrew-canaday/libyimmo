/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 *  Copyright (c) 2014 Andrew Canaday
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




#ifndef YMO_HDR_HASH_H
#define YMO_HDR_HASH_H
#include "yimmo.h"
#include "ymo_http_hdr_table.h"


YMO_FUNC_UNUSED static inline ymo_http_hdr_id_t ymo_http_hdr_hash_227_23(const char* str_in, size_t* len)
{
    const char* hdr_start = str_in;
    char c;
    ymo_http_hdr_id_t h = 23;
    while( (c = *str_in++) ) {
        h = (h*277) + (c & 0xdf);
    }

    if( len ) {
        *len = (size_t)(str_in - hdr_start)-1;
    }
    return h & YMO_HDR_TABLE_MASK;
}

YMO_FUNC_UNUSED static inline ymo_http_hdr_id_t ymo_http_hdr_hash_313_53(const char* str_in, size_t* len)
{
    const char* hdr_start = str_in;
    char c;
    ymo_http_hdr_id_t h = 313;
    while( (c = *str_in++) ) {
        h = (h*53) ^ (c & 0xdf);
    }

    if( len ) {
        *len = (size_t)(str_in - hdr_start)-1;
    }
    return h & YMO_HDR_TABLE_MASK;
}

YMO_FUNC_UNUSED static inline ymo_http_hdr_id_t ymo_http_hdr_hash_311_127(const char* str_in, size_t* len)
{
    const char* hdr_start = str_in;
    char c;
    ymo_http_hdr_id_t h = 127;
    while( (c = *str_in++) ) {
        h = (h*311) ^ (c & 0xdf);
    }

    if( len ) {
        *len = (size_t)(str_in - hdr_start)-1;
    }
    return h & YMO_HDR_TABLE_MASK;
}

#endif /* YMO_HDR_HASH_H */
