#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/errno.h>

#include "ymo_config.h"
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

