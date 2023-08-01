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
#include <stdio.h>
#include <libgen.h>

#include "yimmo.h"
#include "core/ymo_tap.h"
#include "ymo_log.h"
#include "ymo_yaml.h"
#include "core/ymo_tap.h"

static char yaml_path[1024]; /* HACK HACK HACK! */
static ymo_yaml_doc_t* doc = NULL;
static const ymo_yaml_node_t* root = NULL;

static int setup(void)
{
    ymo_log_info("The executable is %s", ymo_tap_exec);
    char test_path[1024]; /* HACK HACK HACK */
    strcpy(test_path, ymo_tap_exec);
    sprintf(yaml_path, "%s/../%s", dirname(test_path), "yaml_test.yml");
    ymo_log_info("The yaml path is: %s", yaml_path);

    return 0;
}


int test_yaml_load_from_path(void)
{
    doc = ymo_yaml_load_file(yaml_path);
    root = ymo_yaml_doc_root(doc);

    ymo_assert(doc != NULL);
    ymo_assert(root != NULL);

#if 0
    /* HACK HACK HACK */
    ymo_yaml_doc_print_json_hack(stderr, doc);
    fprintf(stderr, "%s", "\n");
#endif
    YMO_TAP_PASS(__func__);
}


int test_yaml_nested_obj_string(void)
{
    /* Test nesting objects + string values: */
    const ymo_yaml_node_t* obj = ymo_yaml_object_get(root, "simple_object");
    ymo_assert(ymo_yaml_node_type(obj) == YMO_YAML_OBJECT);
    const ymo_yaml_node_t* val = ymo_yaml_object_get(obj, "key1");
    ymo_assert(ymo_yaml_node_type(val) == YMO_YAML_SCALAR);
    ymo_assert_str_eq(ymo_yaml_node_as_str(val), "val1");
    YMO_TAP_PASS(__func__);
}


int test_yaml_int(void)
{
    /* Test integer values. */
    long simple_integer;
    ymo_assert(ymo_yaml_node_as_long(ymo_yaml_object_get(
            root, "simple_integer"), &simple_integer) == 0);
    ymo_assert(simple_integer == 12345);
    YMO_TAP_PASS(__func__);
}


int test_yaml_int_fmts(void)
{
    const ymo_yaml_node_t* integer_fmts = ymo_yaml_object_get(root, "integer_fmts");

    long val;
    int no_items = 0;
    const ymo_yaml_node_t* item = ymo_yaml_item_next(integer_fmts, NULL);
    ymo_assert(item != NULL);
    while( item ) {
        ++no_items;
        ymo_assert(ymo_yaml_node_as_long(item, &val) == 0);
        ymo_assert(val == 127);
        item = ymo_yaml_item_next(integer_fmts, item);
    }
    YMO_TAP_PASS(__func__);
}


int test_yaml_float(void)
{
    /* Test float values. */
    float simple_float;
    ymo_assert(ymo_yaml_node_as_float(ymo_yaml_object_get(
            root, "simple_float"), &simple_float) == 0);
    ymo_assert(simple_float > 0.706);
    ymo_assert(simple_float < 0.708);
    YMO_TAP_PASS(__func__);
}


int test_yaml_list(void)
{
    const ymo_yaml_node_t* item = NULL;
    const ymo_yaml_node_t* list = ymo_yaml_object_get(root, "simple_list");
    ymo_assert(ymo_yaml_node_type(list) == YMO_YAML_SEQUENCE);

    item = ymo_yaml_item_next(list, NULL);
    ymo_assert_str_eq(ymo_yaml_node_as_str(item), "item_0");

    item = ymo_yaml_item_next(list, item);
    ymo_assert_str_eq(ymo_yaml_node_as_str(item), "item_1");

    item = ymo_yaml_item_next(list, item);
    ymo_assert_str_eq(ymo_yaml_node_as_str(item), "item_2");

    ymo_assert(ymo_yaml_item_next(list, item) == NULL);
    YMO_TAP_PASS(__func__);
}


int test_yaml_free(void)
{
    ymo_yaml_doc_free(doc);
    YMO_TAP_PASS(__func__);
    return 0;
}


YMO_TAP_RUN(setup, NULL, NULL,
        YMO_TAP_TEST_FN(test_yaml_load_from_path),
        YMO_TAP_TEST_FN(test_yaml_nested_obj_string),
        YMO_TAP_TEST_FN(test_yaml_int),
        YMO_TAP_TEST_FN(test_yaml_int_fmts),
        YMO_TAP_TEST_FN(test_yaml_float),
        YMO_TAP_TEST_FN(test_yaml_list),
        YMO_TAP_TEST_FN(test_yaml_free),
        YMO_TAP_TEST_END()
        )


