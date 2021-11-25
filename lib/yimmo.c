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
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "ymo_config.h"
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

typedef struct ymo_version_info {
    union {
        unsigned int  component[3];
        struct {
            unsigned int  major;
            unsigned int  minor;
            unsigned int  patch;
        };
    };
    const char* extra;
} ymo_version_info_t;

const char* ymo_version_str(void)
{
    return VERSION;
}

ymo_version_info_t* get_ymo_version_info(const char* ymo_version)
{
    char* version_str = malloc(strlen(VERSION));
    if( !version_str ) {
        return NULL;
    }

    ymo_version_info_t* version_info = YMO_NEW0(ymo_version_info_t);
    if( version_info ) {
        int i = 0;
        strcpy(version_str, VERSION);
        const char* dash = NULL;
        char* saveptr = NULL;
        char* component = NULL;
        char* endptr;

        if( !version_str ) {
            return version_info;
        }

        for( component = strtok_r(version_str, ".", &saveptr); component;
             component = strtok_r(NULL, ".", &saveptr))
        {
            if( i > 2 ) {
                break;
            }

            unsigned int version =  \
                (unsigned int)strtol(component, &endptr, 10);
            if( *endptr == '\0' && *component != '\0' ) {
                version_info->component[i++] = version;
            }
        }

        dash = strchr(VERSION, '-');
        if( dash != NULL && *dash != '\0' ) {
            version_info->extra = dash+1;
        }
    }

    free(version_str);
    return version_info;
}

void ymo_version_info_free(ymo_version_info_t* version_info)
{
    YMO_DELETE(ymo_version_info_t, version_info);
    return;
}

unsigned int ymo_version_major(void)
{
    unsigned int v_major = 0;
    ymo_version_info_t* version_info = get_ymo_version_info(VERSION);
    v_major = version_info->major;
    ymo_version_info_free(version_info);
    return v_major;
}

unsigned int ymo_version_minor(void)
{
    unsigned int v_minor = 0;
    ymo_version_info_t* version_info = get_ymo_version_info(VERSION);
    v_minor = version_info->minor;
    ymo_version_info_free(version_info);
    return v_minor;
}

unsigned int ymo_version_patch(void)
{
    unsigned int v_patch = 0;
    ymo_version_info_t* version_info = get_ymo_version_info(VERSION);
    v_patch = version_info->patch;
    ymo_version_info_free(version_info);
    return v_patch;
}

const char* ymo_version_extra(void)
{
    const char* v_extra = 0;
    ymo_version_info_t* version_info = get_ymo_version_info(VERSION);
    v_extra = version_info->extra;
    ymo_version_info_free(version_info);
    return v_extra;
}

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
        ymo_log_level_t level = ymo_log_set_level_by_name(log_level);
    }
    return;
}

