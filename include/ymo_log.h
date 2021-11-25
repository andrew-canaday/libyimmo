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



/** Logging
 * ===========
 *
 * Yimmo logging API.
 *
 */

#ifndef YMO_LOG_H
#define YMO_LOG_H
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <uuid/uuid.h>
#include <ev.h>

/** Yimmo log levels.
 *
 * :YMO_LOG_FATAL: An event has occurred which is considered unrecoverable.
 * :YMO_LOG_ERROR: An event has occurred which limits or prevents normal
 *     operation for at least one connection.
 * :YMO_LOG_WARNING: An error may occur if no corrective action is taken.
 * :YMO_LOG_NOTICE: An anomlous or noteworthy event has taken place.
 * :YMO_LOG_INFO: General non-error, user information.
 * :YMO_LOG_DEBUG: General non-error, developer information.
 * :YMO_LOG_TRACE: This is a log level intended almost exclusively for yimmo development.
 *     It provides fine-grained logging on most of the internal functions,
 *     including parsing operations and object creation/destruction.
 */
typedef enum ymo_log_level {
    /** An event has occurred which is considered unrecoverable. */
    YMO_LOG_FATAL,
    /** An event has occurred which limits or prevents normal
     *  operation for at least one connection. */
    YMO_LOG_ERROR,
    /** An error may occur if no corrective action is taken. */
    YMO_LOG_WARNING,
    YMO_LOG_NOTICE, /* An anomlous or noteworthy event has taken place. */
    YMO_LOG_INFO,   /* General non-error, no-developer information. */
    YMO_LOG_DEBUG,  /* General non-error, developer information. */
    /** This is a log level intended almost exclusively for yimmo development.
     *  It provides fine-grained logging on most of the internal functions,
     *  including parsing operations and object creation/destruction. */
    YMO_LOG_TRACE,
} ymo_log_level_t;

/** Initialize the logger (atm, just sets the level from the env var,
 * ``YIMMO_LOG_LEVEL``).
 *
 */
void ymo_log_init(void);

/** Log function
 *
 * :param level: the log level of the current message
 * :param fmt: printf-style format string for subsequent variadic args
 */
void ymo_log(ymo_log_level_t level, const char* fmt, ...);

/** Log function
 *
 * :param level: the log level of the current message
 * :param msg: puts-style log message
 */
void ymo_log_msg(ymo_log_level_t level, const char* msg);

/** Get the current log-level.
 *
 * :returns: the current log level
 */
ymo_log_level_t ymo_log_get_level(void);

/** Get the name for a given log level.
 *
 * :param level: the level whose name we want
 * :returns: the level name in string form
 */
const char* ymo_log_get_level_name(ymo_log_level_t level);

/** Set the current log-level.
 *
 * :param level: the level at which the logger should omit messages
 * :returns: the current log level
 */
ymo_log_level_t ymo_log_set_level(ymo_log_level_t level);

/** Set the current log-level by name, e.g. ymo_log_set_level_by_name("TRACE")
 *
 * :param level: the level at which the logger should omit messages
 * :returns: the current log level
 */
ymo_log_level_t ymo_log_set_level_by_name(const char* level_name);


/*---------------------------------------------------------------------------*
 * Compile-time log macros.
 * These macros allow you to make ymo_log calls at specific levels, and
 * compile them out entirely by defining YMO_LOG_LEVEL_MAX to a value lower
 * than the given log-level.
 *---------------------------------------------------------------------------*/

/** Compile time log-level max. */
#ifndef YMO_LOG_LEVEL_MAX
#define YMO_LOG_LEVEL_MAX 4
#endif /* YMO_LOG_LEVEL_MAX */

#ifndef YMO_LOG_LEVEL_DEFAULT
#define YMO_LOG_LEVEL_DEFAULT 3
#endif /* YMO_LOG_LEVEL_DEFAULT */

#if YMO_LOG_LEVEL_MAX >= 0

/** Compile-time FATAL log function/convenience macro */
#define ymo_log_fatal(...) \
    ymo_log(YMO_LOG_FATAL, __VA_ARGS__)

#else
#define ymo_log_fatal(...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_FATAL */


#if YMO_LOG_LEVEL_MAX >= 1

/** Compile-time ERROR log function/convenience macro */
#define ymo_log_error(...) \
    ymo_log(YMO_LOG_ERROR, __VA_ARGS__)

#else
#define ymo_log_error(...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_ERROR */


#if YMO_LOG_LEVEL_MAX >= 2

/** Compile-time WARNING log function/convenience macro */
#define ymo_log_warning(...) \
    ymo_log(YMO_LOG_WARNING, __VA_ARGS__)

#else
#define ymo_log_warning(...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_WARNING */


#if YMO_LOG_LEVEL_MAX >= 3

/** Compile-time NOTICE log function/convenience macro */
#define ymo_log_notice(...) \
    ymo_log(YMO_LOG_NOTICE, __VA_ARGS__)

#else
#define ymo_log_notice(...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_NOTICE */


#if YMO_LOG_LEVEL_MAX >= 4

/** Compile-time INFO log function/convenience macro */
#define ymo_log_info(...) \
    ymo_log(YMO_LOG_INFO, __VA_ARGS__)

#else
#define ymo_log_info(...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_INFO */


#if YMO_LOG_LEVEL_MAX >= 5
#define ymo_log_debug_uuid(fmt, id, ...) \
    do \
    { \
        char id_str[37]; \
        uuid_unparse(id, id_str); \
        ymo_log(YMO_LOG_DEBUG, "%s:%i (%s): "fmt, \
        __func__, __LINE__, id_str, __VA_ARGS__); \
    } while(0)

/** Compile-time DEBUG log function/convenience macro */
#define ymo_log_debug(fmt, ...) \
    ymo_log(YMO_LOG_DEBUG, "(%i) %s:%i: "fmt, (int)getpid(), __func__, __LINE__, __VA_ARGS__)

#else
#define ymo_log_debug_uuid(fmt, id, ...) ((void)0)
#define ymo_log_debug(fmt, ...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_DEBUG */


#if YMO_LOG_LEVEL_MAX >= 6
#define ymo_log_trace_uuid(fmt, id, ...) \
    do \
    { \
        char id_str[37]; \
        uuid_unparse(id, id_str); \
        ymo_log(YMO_LOG_TRACE, "%s:%i (%s): "fmt, \
        __func__, __LINE__, id_str, __VA_ARGS__); \
    } while(0)

/** Compile-time TRACE log function/convenience macro */
#define ymo_log_trace(fmt, ...) \
    ymo_log(YMO_LOG_TRACE, "%s:%i: "fmt, __func__, __LINE__, __VA_ARGS__)

#else
#define ymo_log_trace_uuid(fmt, id, ...) ((void)0)
#define ymo_log_trace(fmt, ...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_TRACE */


#endif /* YMO_LOG_H */




/*
TODO: count number of va_args and avoid use fputs instead of vfprintf when
the number is zero.

(Stolen from: https://stackoverflow.com/questions/4421681/how-to-count-the-number-of-arguments-passed-to-a-function-that-accepts-a-variabl)

----
You can let the preprocessor help you cheat using this strategy, stolen and tweaked from another answer:

#include <stdio.h>
#include <stdarg.h>

#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_128TH_ARG(__VA_ARGS__)
#define PP_128TH_ARG( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
         _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
         _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
         _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
         _101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
         _111,_112,_113,_114,_115,_116,_117,_118,_119,_120, \
         _121,_122,_123,_124,_125,_126,_127,N,...) N
#define PP_RSEQ_N() \
         127,126,125,124,123,122,121,120, \
         119,118,117,116,115,114,113,112,111,110, \
         109,108,107,106,105,104,103,102,101,100, \
         99,98,97,96,95,94,93,92,91,90, \
         89,88,87,86,85,84,83,82,81,80, \
         79,78,77,76,75,74,73,72,71,70, \
         69,68,67,66,65,64,63,62,61,60, \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

void _variad(size_t argc, ...);
#define variad(...) _variad(PP_NARG(__VA_ARGS__), __VA_ARGS__)

void _variad(size_t argc, ...) {
    va_list ap;
    va_start(ap, argc);
    for (int i = 0; i < argc; i++) {
        printf("%d ", va_arg(ap, int));
    }
    printf("\n");
    va_end(ap);
}

int main(int argc, char* argv[]) {
    variad(2, 4, 6, 8, 10);
    return 0;
}

There's a few clever tricks here.

1) Instead of calling the variadic function directly, you're calling a macro that counts the arguments and passes the argument count as the first argument to the function. The end result of the preprocessor on main looks like:

_variad(5, 2, 4, 6, 8, 10);

2) PP_NARG is a clever macro to count arguments.

The workhorse here is PP_128TH_ARG. It returns its 128th argument, by ignoring the first 127 arguments (named arbitrarily _1 _2 _3 etc.), naming the 128th argument N, and defining the result of the macro to be N.

PP_NARG invokes PP_128TH_ARG with __VA_ARGS__ concatenated with PP_RSEQ_N, a reversed sequence of numbers counting from 127 down to 0.

If you provide no arguments, the 128th value of PP_RSEQ_N is 0. If you pass one argument to PP_NARG, then that argument will be passed to PP_128TH_ARG as _1; _2 will be 127, and the 128th argument to PP_128TH_ARG will be 1. Thus, each argument in __VA_ARGS__ bumps PP_RSEQ_N over by one, leaving the correct answer in the 128th slot.

(Apparently 127 arguments is the maximum C allows.)
*/
