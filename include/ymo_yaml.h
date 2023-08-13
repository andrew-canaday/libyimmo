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



#ifndef YMO_YAML_H
#define YMO_YAML_H
#include <stdio.h>
#include <stdarg.h>

/** Yimmo YAML
 * ==============
 *
 * warning:: This module is a hack!
 *
 */


/** YMO Yaml node type. */
typedef enum {
    YMO_YAML_ERROR,
    YMO_YAML_OBJECT,
    YMO_YAML_SCALAR,
    YMO_YAML_SEQUENCE,
} ymo_yaml_type_t;

/** Opaque type reprsenting a yaml doc. */
typedef struct ymo_yaml_doc ymo_yaml_doc_t;

/** Opaque type reprsenting a yaml node. */
typedef struct ymo_yaml_node ymo_yaml_node_t;


/** Doc Functions
 * -------------
 */

/** Load a yaml document from the given file path.
 *
 * TODO: Probably, it'd be nice to be able to load from FILE* or memory too.
 *
 * :param f_path: the file to load the yaml from
 * :returns: pointer to yaml doc; else return ``null`` and set ``errno``
 */
ymo_yaml_doc_t* ymo_yaml_load_file(const char* f_path);

/** Get the file path for a loaded yaml doc. */
const char* ymo_yaml_doc_filepath(const ymo_yaml_doc_t* doc);

/** Get the root node for a YAML document. */
ymo_yaml_node_t* ymo_yaml_doc_root(const ymo_yaml_doc_t* doc);

/** Added for debug. Probably, don't use this. It's going away (or else
 * going to be significantly refactored.
 */
void ymo_yaml_doc_print_json_hack(FILE* out_file, const ymo_yaml_doc_t* doc);

/** Free a yaml doc. */
void ymo_yaml_doc_free(ymo_yaml_doc_t* doc);


/** Node Functions
 * --------------
 *
 * TODO: scalar, sequence, and type conversion stuff ala ymo_env.
 *
 *---------------------------------------------------------------*/

/** Given a yaml item, return its type. */
ymo_yaml_type_t ymo_yaml_node_type(const ymo_yaml_node_t* node);

/** Iterate over items in a list or keys in a mapping, one by one.
 * Invoked the first time with current = NULL;
 */
const ymo_yaml_node_t* ymo_yaml_item_next(const ymo_yaml_node_t* list, const ymo_yaml_node_t* current);

/** Given a set of keys, return the value of the first yaml node that matches. */
const ymo_yaml_node_t* ymo_yaml_doc_get(ymo_yaml_doc_t* doc, ...);

/** Given a set of keys, return the value of the first yaml node that matches. */
const ymo_yaml_node_t* ymo_yaml_doc_vget(ymo_yaml_doc_t* doc, va_list args);

/** Given a yaml object, return the value for the given key as a yaml node. */
const ymo_yaml_node_t* ymo_yaml_object_get(const ymo_yaml_node_t* node, const char* key);

/** Given a yaml node representing a mapping key, return the value node. */
const ymo_yaml_node_t* ymo_yaml_node_child(const ymo_yaml_node_t* key);

/** Given a yaml object, return the value as a string. */
const char* ymo_yaml_node_as_str(const ymo_yaml_node_t* node);

/** Given a yaml object, return the value as a long. */
int ymo_yaml_node_as_long(const ymo_yaml_node_t* node, long* value);

/** Given a yaml object, return the value as a float. */
int ymo_yaml_node_as_float(const ymo_yaml_node_t* node, float* value);

#endif /* YMO_YAML_H */

