/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 * ymo_assert.h: Convenience macros for unit test_fn assertions
 *
 * Copyright (c) 2014 Andrew Canaday
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



/** TAP (Test Anything Protocol)
 * ==============================
 *
 * .. warning::
 *   *This file must be included before (or instead of) ``ymo_assert.h``.*
 *
 */

#ifndef YMO_TAP_H
#define YMO_TAP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef YMO_SOURCE
#  ifdef __FILE_NAME__
#    define YMO_SOURCE __FILE_NAME__
#  else
#    define YMO_SOURCE __FILE__
#  endif /* __FILE_NAME__ */
#endif /* YMO_SOURCE */

#define YMO_ASSERT_STREAM_OUT   stderr

/** Output stream for TAP results.
 */
#define YMO_TAP_STREAM_OUT      stdout

/** Value to return from passing unit tests.
 *
 * (**NOTE**: :c:macro:`YMO_TAP_PASS` is preferred)
 */
#define YMO_TAP_STATUS_PASS 0


/** Value to return from failing unit tests (any non-zero value
 * will work — ``errno`` is always a safe bet).
 *
 * (**NOTE**: :c:macro:`YMO_TAP_FAIL` is preferred)
 */
#define YMO_TAP_STATUS_FAIL 1

/* Override ymo_assert_test_abort to return YMO_TAP_STATUS_FAIL */
#define ymo_assert_test_abort() return YMO_TAP_STATUS_FAIL

/* Override ymo_assert_test_pass to no-op */
#define ymo_assert_test_pass(msg)

/** This module overrides :c:macro:`ymo_assert_test_fail` to return
 * :c:macro:`YMO_TAP_STATUS_FAIL`.
 *
 * The assertion conditional will be reported as test output, e.g.:
 *
 * .. code-block:: c
 *
 *    int x = 1;
 *    int y = 2;
 *    ymo_assert(x == y);
 *
 *
 * Will result in:
 *
 * .. code-block::
 *
 *    not ok 1 - assertion failed!
 *       assertion: x == y
 *       location:  my_source.c:test_my_test:3
 *
 * The override (included in ymo_tap.h!) looks like this:
 */

#define ymo_assert_test_fail(msg) ymo_tap_assert_fail(msg)

#define ymo_tap_assert_fail(msg) \
    fprintf(YMO_TAP_STREAM_OUT, \
        "not ok %zu - assertion failed!\n   " \
        "assertion: %s\n   " \
        "location:  %s:%s:%i\n", \
        tap_test_num, msg, \
        YMO_SOURCE, __func__, __LINE__); \
    return YMO_TAP_STATUS_FAIL


/** Pass a unit test with the given message.
 */
#define YMO_TAP_PASS(msg) \
    fprintf(YMO_TAP_STREAM_OUT, "ok %zu - %s\n", \
        tap_test_num, msg); \
    return YMO_TAP_STATUS_PASS


/** Pass a unit test with the given message.
 */
#define YMO_TAP_FAIL(msg) \
    fprintf(YMO_TAP_STREAM_OUT, "not ok %zu - %s\n", \
        tap_test_num, msg); \
    return YMO_TAP_STATUS_FAIL

#include "ymo_assert.h"

static size_t tap_test_num = 0;

/** Individual unit test signature.
 *
 */
typedef int (* ymo_tap_test_fn_t)(void);

/** Struct used to represent individual unit tests.
 */
typedef struct ymo_tap_test ymo_tap_test_t;

struct ymo_tap_test {
    const char*        name;
    ymo_tap_test_fn_t  test_fn;
};


/** Macro used to pass a :c:type:`ymo_test_fn_t` as an argument to
 * :c:func:`ymo_tap_run` or :c:macro:`YMO_TAP_RUN`.
 *
 */
#define YMO_TAP_TEST_FN(fn) \
    ((ymo_tap_test_t) { .name = #fn, .test_fn = fn })

/** Macro used to terminate the list of arguments to :c:func:`ymo_tap_run`
 * or :c:macro:`YMO_TAP_RUN`.
 *
 */
#define YMO_TAP_TEST_END() \
    ((ymo_tap_test_t) { .name = NULL, .test_fn = NULL })


/** :param ymo_tap_test_fn_t: a function to be run once at
 *     initialization time (i.e. once per test *program*,
 *     not once per *function*).
 * :param ...: a `NULL` terminated list of :c:type:`ymo_tap_test_t`.
 * :returns: zero on success; non-zero on failure
 *
 */
int ymo_tap_run(ymo_tap_test_fn_t test_init, ...);


int ymo_tap_run(ymo_tap_test_fn_t test_init, ...)
{
    va_list ap;
    va_list ap2;

    size_t no_tests = 0;

    ymo_tap_test_t tap_test;

    va_start(ap, test_init);
    va_copy(ap2, ap);

    /* Count the tests: */
    do {
        tap_test = va_arg(ap, ymo_tap_test_t);
        if( tap_test.test_fn ) {
            no_tests++;
        } else {
            break;
        }
    } while(tap_test.test_fn);
    va_end(ap);

    if( test_init ) {
        int init_status = test_init();
        if( init_status ) {
            return init_status;
        }
    }

    /* Run them: */
    int suite_status = 0;
    fprintf(YMO_TAP_STREAM_OUT, "1..%zu\n", no_tests);
    for( tap_test_num = 1; tap_test_num <= no_tests; tap_test_num++ ) {
        tap_test = va_arg(ap2, ymo_tap_test_t);
        if( tap_test.test_fn() ) {
            suite_status = -1;
        }
    }
    va_end(ap2);
    return suite_status;
}

/** .. c:macro::YMO_TAP_RUN(...)
 *
 * Define a ``main`` function which invokes ::c:func:`ymo_tap_run` with
 * the input arguments.
 *
 */
#define YMO_TAP_RUN(...) \
    int main(int argc, char** argv) { \
        return ymo_tap_run(__VA_ARGS__); \
    }

#endif /* YMO_TAP_H */
