/*=============================================================================
 * test/ymo_basic: Test basic functions of libyimmo.
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

/* HACK HACK: for strdup */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "yimmo.h"
#include "ymo_util.h"
#include "core/ymo_tap.h"

char* naive_tolower(char* str_in)
{
    size_t i;
    for( i = 0; i < strlen(str_in); ++i )
    {
        str_in[i] = tolower(str_in[i]);
    }
    return str_in;
}


char* naive_toupper(char* str_in)
{
    size_t i;
    for( i = 0; i < strlen(str_in); ++i )
    {
        str_in[i] = toupper(str_in[i]);
    }
    return str_in;
}


const char* s_orig = "This is a test (Copyright blah blah BLAH!)";
char* lower_expect = NULL;
char* upper_expect = NULL;
char* s_lower = NULL;
char* s_upper = NULL;
size_t s_len = 0;

int setup(void)
{
    static int setup_done = 0;

    if( !setup_done ) {
        setup_done = 1;
        lower_expect = naive_tolower(strdup(s_orig));
        upper_expect = naive_toupper(strdup(s_orig));
    }
    return 0;
}


int test_to_lower(void)
{
    s_lower = strdup(s_orig);
    s_upper = strdup(s_orig);
    s_len = strlen(s_lower);

    /* Test once, the normal way: */
    ymo_ntolower(s_lower, s_lower, strlen(s_lower));
    ymo_ntoupper(s_upper, s_upper, strlen(s_upper));
    ymo_assert_str_eq(s_lower, lower_expect);
    YMO_TAP_PASS(__func__);
}


int test_trim_front(void)
{
    size_t i;
    /* Test again, trimming letters off the front: */
    for( i = 0; i < s_len; ++i )
    {
        /* lower: */
        strcpy(s_lower, s_orig);
        ymo_ntolower(s_lower+i, s_lower+i, strlen(s_lower+i));
        ymo_assert_str_eq(s_lower+i, lower_expect+i);

        /* upper: */
        strcpy(s_upper, s_orig);
        ymo_ntoupper(s_upper+i, s_upper+i, strlen(s_upper+i));
        ymo_assert_str_eq(s_upper+i, upper_expect+i);
    }

    /* Test again, trimming letters off the back: */
    for( i = s_len; i > 0; --i )
    {
        /* lower: */
        strcpy(s_lower, s_orig);
        s_lower[i] = '\0';
        lower_expect[i] = '\0';
        ymo_ntolower(s_lower, s_lower, strlen(s_lower));
        ymo_assert_str_eq(s_lower, lower_expect);

        /* upper: */
        strcpy(s_upper, s_orig);
        s_upper[i] = '\0';
        upper_expect[i] = '\0';
        ymo_ntoupper(s_upper, s_upper, strlen(s_upper));
        ymo_assert_str_eq(s_upper, upper_expect);
    }

    YMO_TAP_PASS(__func__);
}


int test_base64_encoded(void)
{
    const char* s_in = "This is a string of text";
    char* s_b64_out = ymo_base64_encoded(s_in, strlen(s_in));
    ymo_assert_str_eq("VGhpcyBpcyBhIHN0cmluZyBvZiB0ZXh0", s_b64_out);
    YMO_TAP_PASS(__func__);
}


#define assert_utf8_value(s, d, v) \
        do { \
            ymo_utf8_state_t state; \
            ymo_utf8_state_reset(&state); \
            int rc = ymo_check_utf8(&state, s, sizeof(s)-1, d); \
            ymo_assert_test(rc == v, "Expected " #v " for " #s); \
        } while( 0 );

#define assert_utf8_valid(s, d) \
        assert_utf8_value(s, d, YMO_OKAY)

#define assert_utf8_invalid(s, d) \
        assert_utf8_value(s, d, EILSEQ)

int test_utf8_validation(void)
{
    ymo_log_init();

    /* Valid: */
    assert_utf8_valid("Ascii is okay when done.", 1);
    assert_utf8_valid("Ascii is okay when not done.", 0);
    assert_utf8_valid("\x48\x65\x6c\x6c\x6f\x2d\xc2\xb5\x40\xc3\x9f\xc3\xb6\xc3\xa4\xc3\xbc\xc3\xa0\xc3\xa1\x2d\x55\x54\x46\x2d\x38\x21\x21", 1);
    assert_utf8_valid("Incomplete, bud not done: \xf4\x8f", 0);

    /* Invalid: */
    assert_utf8_invalid("This should fail due to: \xff.", 1);
    assert_utf8_invalid("Invalid start: \xc0.", 1);
    assert_utf8_invalid("Invalid start: \xc1.", 1);
    assert_utf8_invalid("Invalid continuation: \xdf\xc0", 1);
    assert_utf8_invalid("Invalid continuation: \xe0\xc0\xc0", 1);
    assert_utf8_invalid("Invalid continuation: \xf0\xc0\xc0\xc0", 1);
    assert_utf8_invalid("This has a UTF-16 surrogate pair: \xed\xa1.", 1);
    assert_utf8_invalid("Overlong: \xe0\x9f\x80", 1);
    assert_utf8_invalid("Overlong: \xf0\x8f\x80\x80", 1);
    assert_utf8_invalid("Out of range: \xf4\x8f\xbf\xc0", 1);
    assert_utf8_invalid("Incomplete and done: \xf4\x8f", 1);

    YMO_TAP_PASS(__func__);
}


YMO_TAP_RUN(setup, NULL, NULL,
        YMO_TAP_TEST_FN(test_to_lower),
        YMO_TAP_TEST_FN(test_trim_front),
        YMO_TAP_TEST_FN(test_base64_encoded),
        YMO_TAP_TEST_FN(test_utf8_validation),
        YMO_TAP_TEST_END()
        )

