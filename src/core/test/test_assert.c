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

#ifdef YMO_ASSERT_VERBOSE
#undef YMO_ASSERT_VERBOSE
#endif /* YMO_ASSERT_VERBOSE */

/* Force verbose for the assertion demo: */
#define YMO_ASSERT_VERBOSE 1

#include "core/ymo_assert.h"

int main(int argc, char** argv)
{
    int two = 2;
    int three = 3;
    const char* this_str = "This";
    const char* this_str_cpy = strdup(this_str);
    const char* that_str = "That";
    const char* null_str = NULL;
    const char* haystack = "prefix string";
    const char* needle = "pre";

    puts("\nStandard assertions:");
    ymo_assert(two < three);
    ymo_assert(10 > 1);
    ymo_assert(1 <= two);
    ymo_assert(two <= two);
    ymo_assert(three >= two);
    ymo_assert(three >= three);
    ymo_assert( ((164 % 100) & 0x40) == ((6-4) << 5) );
    ymo_assert(two == two);
    ymo_assert(two != three);

    puts("\nString asertions:");
    ymo_assert_str_eq("This", this_str);
    ymo_assert_str_eq(that_str, strdup(that_str));
    ymo_assert_str_eq(this_str, this_str_cpy);
    ymo_assert_str_eq(null_str, NULL);
    ymo_assert_str_ne(this_str, that_str);
    ymo_assert_str_ne(this_str, null_str);
    ymo_assert_str_ne(this_str, NULL);
    ymo_assert_str_contains("the subway is neat", "sub");
    ymo_assert_str_startswith(haystack, needle);
    return 0;
}

