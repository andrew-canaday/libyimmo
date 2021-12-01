/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 * ymo_assert.h: Convenience macros for unit test assertions
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



#ifndef YMO_ASSERT_H
#define YMO_ASSERT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef YMO_ASSERT_VERBOSE
#define YMO_ASSERT_VERBOSE 0
#endif /* YMO_ASSERT_VERBOSE */

#ifndef YMO_ASSERT_STREAM_OUT

/** Output stream for assertions */
#define YMO_ASSERT_STREAM_OUT stdout

#endif /* YMO_ASSERT_STREAM_OUT */

#ifndef YMO_SOURCE
#  ifdef __FILE_NAME__
#    define YMO_SOURCE __FILE_NAME__
#  else
#    define YMO_SOURCE __FILE__
#  endif /* __FILE_NAME__ */
#endif /* YMO_SOURCE */

/** Assertions
 * =============
 */

/**--------------------------------------------------
 *             Convenience Macros
 *--------------------------------------------------*/

/*--- ymo_assert_test_abort: ---*/
#ifndef ymo_assert_test_abort

/** Function used to abort test execution
 */
#define ymo_assert_test_abort() exit(-1)

#endif /* ymo_assert_test_fail */


/*--- ymo_assert_test_fail_fmt: ---*/
#ifndef ymo_assert_test_fail_fmt

/** Function used to report assertion failure and abort
 */
#define ymo_assert_test_fail_fmt(fmt, ...) \
    fprintf(YMO_ASSERT_STREAM_OUT, \
        " - \033[00;31mFAIL: "fmt " (%s:%s:%i)\033[00;m\n", \
        __VA_ARGS__, YMO_SOURCE, __func__, __LINE__); \
    ymo_assert_test_abort();

#endif /* ymo_assert_test_fail_fmt */


/*--- ymo_assert_test_fail: ---*/
#ifndef ymo_assert_test_fail

/** Function used to report assertion failure and abort
 */
#define ymo_assert_test_fail(test_desc) \
    ymo_assert_test_fail_fmt("%s", test_desc); \
    ymo_assert_test_abort();

#endif /* ymo_assert_test_fail */


/*--- ymo_assert_test_pass_fmt: ---*/
#ifndef ymo_assert_test_pass_fmt

/** Function used to report assertion success, if verbose is defined
 */
#define ymo_assert_test_pass_fmt(fmt, ...) \
    if( YMO_ASSERT_VERBOSE ) { \
    fprintf(YMO_ASSERT_STREAM_OUT, \
        " - \033[00;32mPASS: "fmt " (%s:%s:%i)\033[00;m\n", \
        __VA_ARGS__, YMO_SOURCE, __func__, __LINE__); \
    }

#endif /* ymo_asser_test_pass_fmt */


/*--- ymo_assert_test_pass: ---*/
#ifndef ymo_assert_test_pass

/** Function used to report assertion success, if verbose is defined
 */
#define ymo_assert_test_pass(test_desc) ymo_assert_test_pass_fmt("%s", test_desc)

#endif /* ymo_assert_test_pass */


/*--- ymo_assert_test_fmt: ---*/
#ifndef ymo_assert_test_fmt
#ifndef NDEBUG

/** Convenience macro used to execute tests and report failure
 */
#define ymo_assert_test_fmt(test_cond, fmt, ...) \
    do { \
        if( !(test_cond) ) { \
            ymo_assert_test_fail_fmt(fmt, __VA_ARGS__); \
        }; \
        ymo_assert_test_pass_fmt(fmt, __VA_ARGS__); \
    } while( 0 )

#else
#define ymo_assert_test_fmt(test_cond, fmt, ...) ((void)0)
#endif /* NDEBUG */
#endif /* ymo_assert_test_fmt */

#ifndef ymo_assert_test
#ifndef NDEBUG


/*--- ymo_assert_test: ---*/
/** Convenience macro used to execute tests and report failure
 */
#define ymo_assert_test(test_cond, test_desc) \
    do { \
        if( !(test_cond) ) { \
            ymo_assert_test_fail(test_desc); \
        }; \
        ymo_assert_test_pass(test_desc); \
    } while( 0 )

#else
#define ymo_assert_test(test_cond, test_desc) ((void)0)
#endif /* NDEBUG */
#endif /* ymo_assert_test */


/**-------------------------------------
 *             Assertions
 *-------------------------------------*/

#ifndef ymo_assert

/** General true/false assertion. */
#define ymo_assert(test_cond) ymo_assert_test(test_cond, #test_cond)

#endif /* ymo_assert */

/** String Assertions
 * ...................
 */

/** String equality:
 *
 * - pass if two non-null strings are lexicographically equal
 * - pass if both strings are the NULL pointer
 */
#define ymo_assert_str_eq(x, y) ymo_assert_test_fmt( \
        (x && y \
         && !strcmp((const char*)x,(const char*)y)) \
        || ((x == NULL) && (y == NULL)), \
        "%s:\n%s: >>%s<<\n%s: >>%s<<\n", \
        #x " == " #y, \
        #x, (const char*)x, \
        #y, (const char*)y \
        )

/** String inequality
 *
 * - pass if two non-null strings are not lexicographically equal
 * - pass if one string is NULL and the other isn't
 */
#define ymo_assert_str_ne(x, y) ymo_assert_test_fmt( \
        (x && y && strcmp((const char*)x,(const char*)y)) \
        || ((x == NULL || y == NULL) && (x != y)), \
        "%s:\n%s: >>%s<<\n%s: >>%s<<\n", \
        #x " != " #y, #x, (const char*)x, #y, (const char*)y)

/** Substring matching: pass if x is a substring of y */
#define ymo_assert_str_contains(x, y) ymo_assert_test( \
        x && y && strstr(x, y) != NULL, #x " contains " #y)

/** Prefix matching: pass if x starts with y */
#define ymo_assert_str_startswith(x, y) ymo_assert_test( \
        x && y && ((const char*)strstr(x, y)) == (const char*)x, \
        #x " starts with " #y)

#endif /* YMO_ASSERT_H */


