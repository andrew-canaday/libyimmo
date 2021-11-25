/*=============================================================================
 * libyimmo: Lightweight socket server framework
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




/** Test Utilities
 * ================
 *
 */

#ifndef YMO_TEST_H
#define YMO_TEST_H

#include "ymo_config.h"
#include <stdlib.h>
#include "yimmo.h"


/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/** Standard test function signature. */
typedef int (* ymo_test_fn_t)(void);

/** Structure used to hold information for individual tests. */
typedef struct ymo_test {
    const char*    name;    /* Test name */
    ymo_test_fn_t  test_fn; /* Test function pointer */
} ymo_test_t;

/**---------------------------------------------------------------
 * Macros
 *---------------------------------------------------------------*/

/** Macro used to declare global test variables.
 */
#define YMO_TEST_DECL_VAR static

/** Macros used inside of tests to indicate pass/fail.
 */
#define YMO_TEST_PASS() return 1
#define YMO_TEST_FAIL() return 0

/** Macro used to declare a test.
 */
#define YMO_TEST_DECLARE(test_name) \
    int test_name(void)

/** Macro used to run an individual ymo_test_t.
 */
#define YMO_TEST_RUN(test_item) \
    printf("\033[00;34;mExecuting test \"%s\"...\033[00;m\n", test_item->name); \
    if( test_item->test_fn() ) { \
        ++ymo_test_no_passed; \
        puts("OK!"); \
    } \
    else { \
        puts("FAIL"); \
    };

/* Macro used to declare a test suite. */
#define YMO_TEST_SUITE_DECLARE(suite_name) \
    ymo_test_t suite_name[] = {

/* Macro used to add a test case to a suite. */
#define YMO_TEST(test_name) \
    { .name = #test_name, .test_fn = test_name },

/* Macro used to finish a suite declaration. */
#define YMO_TEST_SUITE_END \
    {.name = NULL, .test_fn = NULL} \
    };

/** Called once from main to initialize test variables.
 */
#define YMO_TEST_INIT() \
    size_t ymo_test_total = 0; \
    size_t ymo_tests_total_passed = 0; \
    size_t ymo_test_no_passed = 0; \
    size_t ymo_test_no_tests = 0; \
    size_t ymo_test_idx = 0; \
    ymo_test_t* ymo_test_current = NULL;

/** Macro used to complete testing for a translation unit.
 */
#define YMO_TEST_COMPLETE() \
    printf("\nTotal tests passed for all suites: %lu/%lu\n", \
        ymo_tests_total_passed, ymo_test_total);

/** Macro used to run a test suite.
 */
#define YMO_TEST_SUITE_RUN(suite_name) \
    ymo_test_no_passed = 0; \
    ymo_test_idx = 0; \
    ymo_test_no_tests = (sizeof(suite_name) / sizeof(ymo_test_t))-1; \
    ymo_test_total += ymo_test_no_tests; \
    \
    printf("\n===== Test Suite \"%s\" =====\n", #suite_name); \
    printf("Number of tests: %lu\n", ymo_test_no_tests); \
    \
    ymo_test_current = &suite_name[ymo_test_idx++]; \
    while( ymo_test_current->test_fn != NULL ) { \
        YMO_TEST_RUN(ymo_test_current); \
        ymo_test_current = &suite_name[ymo_test_idx++]; \
    }; \
    \
    ymo_tests_total_passed += ymo_test_no_passed; \
    printf("%s: %lu/%lu passed\n", \
        #suite_name, ymo_test_no_passed, ymo_test_no_tests);


#endif /* YMO_TEST_H */



