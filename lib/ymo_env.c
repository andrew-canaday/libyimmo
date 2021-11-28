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
#include <stdint.h>
#include <inttypes.h>
#include <sys/errno.h>

#include "yimmo.h"

int ymo_env_as_long(const char* env_name, long* value, long* def_val)
{
    if( !env_name ) {
        return EINVAL;
    }

    const char* env_s = getenv(env_name);
    if( !env_s ) {
        goto long_env_fail;
    }

    char* endptr;
    long env_l = strtol(env_s, &endptr, 10);
    if( *endptr == '\0' ) {
        *value = env_l;
        return 0;
    }

long_env_fail:
    if( def_val ) {
        *value = *def_val;
        return 0;
    }
    return EINVAL;
}

int ymo_env_as_float(const char* env_name, float* value, float* def_val)
{
    if( !env_name ) {
        return EINVAL;
    }

    const char* env_s = getenv(env_name);
    if( !env_s ) {
        goto float_env_fail;
    }

    char* endptr;
    float env_f = strtof(env_s, &endptr);
    if( *endptr == '\0' ) {
        *value = env_f;
        return 0;
    }

float_env_fail:
    if( def_val ) {
        *value = *def_val;
        return 0;
    }
    return EINVAL;
}

int ymo_env_as_double(const char* env_name, double* value, double* def_val)
{
    if( !env_name ) {
        return EINVAL;
    }

    const char* env_s = getenv(env_name);
    if( !env_s ) {
        goto double_env_fail;
    }

    char* endptr;
    double env_d = strtof(env_s, &endptr);
    if( *endptr == '\0' ) {
        *value = env_d;
        return 0;
    }

double_env_fail:
    if( def_val ) {
        *value = *def_val;
        return 0;
    }
    return EINVAL;
}

