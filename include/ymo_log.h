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
 * .. warning::
 *
 *    The log API and format are more tailored to libyimmo development than
 *    they are to development of projects which *use* libyimmo. Tidying this
 *    up is on the TODO list.
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
    } while( 0 )

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
    } while( 0 )

/** Compile-time TRACE log function/convenience macro */
#define ymo_log_trace(fmt, ...) \
    ymo_log(YMO_LOG_TRACE, "%s:%i: "fmt, __func__, __LINE__, __VA_ARGS__)

#else
#define ymo_log_trace_uuid(fmt, id, ...) ((void)0)
#define ymo_log_trace(fmt, ...) ((void)0)
#endif /* YMO_LOG_LEVEL_MAX >= YMO_LOG_TRACE */


#endif /* YMO_LOG_H */

