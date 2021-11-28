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


#define _GNU_SOURCE
#include "ymo_config.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_alloc.h"

static ymo_log_level_t ymo_global_ll = YMO_LOG_LEVEL_DEFAULT;
static const char* LEVEL_NAMES [] = {
    "FATAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG",
    "TRACE",
};

ymo_log_level_t ymo_log_get_level(void)
{
    return ymo_global_ll;
}

const char* ymo_log_get_level_name(ymo_log_level_t level)
{
    const char* name = "USER";
    if( level <= YMO_LOG_TRACE ) {
        name = LEVEL_NAMES[level];
    }
    return name;
}


ymo_log_level_t ymo_log_set_level(ymo_log_level_t level)
{
    if( level <= YMO_LOG_LEVEL_MAX ) {
        ymo_global_ll = level;
    }
    return ymo_global_ll;
}


ymo_log_level_t ymo_log_set_level_by_name(const char* level_name)
{
    for( int i = 0; i <= YMO_LOG_LEVEL_MAX; i++ ) {
        if( strcasecmp(level_name, LEVEL_NAMES[i]) == 0 ) {
            ymo_log_set_level(i);
        }
    }
    return ymo_global_ll;
}

void ymo_log(ymo_log_level_t level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if( level <= ymo_global_ll ) {
        fprintf(stderr, "yimmo %s: ", LEVEL_NAMES[level]);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }
    va_end(args);
    return;
}

void ymo_log_msg(ymo_log_level_t level, const char* msg)
{
    if( level <= ymo_global_ll ) {
        fprintf(stderr, "yimmo %s: %s\n", LEVEL_NAMES[level], msg);
    }
    return;
}

void ymo_log_init(void)
{
    char* log_level = getenv("YIMMO_LOG_LEVEL");
    if( log_level ) {
        ymo_log_set_level_by_name(log_level);
    }
    return;
}

