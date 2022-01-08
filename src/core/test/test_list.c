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

#include "ymo_config.h"
#include "yimmo.h"
#include "ymo_alloc.h"
#include "ymo_list.h"
#include "core/ymo_tap.h"


typedef struct lletter {
    YMO_LIST_HEAD();
    char  c;
} lletter_t;


int setup(void)
{
    ymo_log_init();
    return YMO_OKAY;
}


int test_list_next(void)
{
    lletter_t* a = YMO_NEW0(lletter_t);
    lletter_t* b = YMO_NEW0(lletter_t);

    /*
    YMO_LIST_INIT(&a);
    YMO_LIST_INIT(&b);
    */

    a->c = 'a';
    b->c = 'b';

    YMO_LIST_APPEND(a, b);
    ymo_assert(YMO_LIST_NEXT(a, lletter_t)->c == 'b');
    ymo_assert(YMO_LIST_PREV(b, lletter_t)->c == 'a');
    ymo_assert(YMO_LIST_PREV(YMO_LIST_NEXT(a, lletter_t), lletter_t)->c == 'a');

    YMO_DELETE(lletter_t, a);
    YMO_DELETE(lletter_t, b);
    YMO_TAP_PASS(__func__);
}


YMO_TAP_RUN(setup, NULL, NULL,
        YMO_TAP_TEST_FN(test_list_next),
        YMO_TAP_TEST_END()
        )

