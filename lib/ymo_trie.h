/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 *  Copyright (c) 2014 Andrew Canaday
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

#ifndef YMO_TRIE_H
#define YMO_TRIE_H

#include "ymo_config.h"

#include <stdint.h>
#include <stdio.h>


#include "yimmo.h"

/** Offset-Indexed TRIE's
 * ========================
 *
 */

/*---------------------
 * Bounds:
 *---------------------*/
#define YMO_OITRIE_8BIT
#ifdef YMO_OITRIE_8BIT

# define YMO_OITRIE_MAX_OFFSET  0xff
# define YMO_TRIE_NO_CHILDREN   0x100
typedef uint8_t ymo_trie_id_t;
typedef uint8_t ymo_oitrie_offset_t;

#else
# define YMO_OITRIE_MAX_OFFSET  0xffff
# define YMO_TRIE_NO_CHILDREN   0x10000
typedef uint16_t ymo_trie_id_t;
typedef uint16_t ymo_oitrie_offset_t;

#endif /* YMO_OITRIE_8BIT */

/*---------------------
 * Flags:
 *---------------------*/
#define OITRIE_HAS_CHILD 0x01
#define OITRIE_IS_TERMINAL 0x10


/*---------------------------------------------------------------*
 *  Types
 *---------------------------------------------------------------*/

/** Typedef forward declarations for translation units that don't need defs: */
typedef struct ymo_trie_node ymo_trie_node_t;
typedef struct ymo_trie ymo_trie_t;
typedef struct ymo_oitrie_node ymo_oitrie_node_t;
typedef struct ymo_oitrie ymo_oitrie_t;

/** Node type for an uncompressed trie. */
struct ymo_trie_node {
    struct ymo_trie_node* children[YMO_TRIE_NO_CHILDREN];
    size_t                index;
    size_t                value_id;
    unsigned char         value;
};

/** Uncompressed trie type.
 * Used to *construct* lookup tables, but not to perform lookups. */
struct ymo_trie {
    ymo_trie_node_t* root;
    size_t           no_nodes;
    size_t           no_values;
};

/** Compressed trie node type. */
struct ymo_oitrie_node {
    uint8_t              flags;
    ymo_trie_id_t        hdr_id;
    ymo_oitrie_offset_t  offset;
    unsigned char        value; /* Char value at this node */
};

struct ymo_oitrie {
    ymo_oitrie_node_t* root;
    size_t             no_nodes;
    size_t             no_values;
};

/*---------------------------------------------------------------*
 *  Functions
 *---------------------------------------------------------------*/

/** Create an uncompressed trie. */
ymo_trie_t* ymo_trie_create(void);

/** Release an umcompressed trie. */
void ymo_trie_free(ymo_trie_t* root);

/** Add a string to the trie. */
ymo_status_t ymo_trie_add_string(
        ymo_trie_t* trie, const char* str_in);

/** Create a node for an uncompressed trie. */
ymo_trie_node_t* ymo_trie_node_create(void);

/** Free an uncompressed child node and *all* it's child nodes. */
void ymo_trie_node_free(ymo_trie_node_t* root);

/** Create a compressed trie from an uncomprssed trie.
 *
 * .. note::
 *     This is a destructive action. All the nodes in the input trie are freed.
 *
 */
ymo_oitrie_t* ymo_oitrie_create(ymo_trie_t* trie);

/** Return the total number of bytes consumed by the oitrie. */
size_t ymo_oitrie_sizeof(const ymo_oitrie_t* oitrie);

/** Free a compressed trie. */
void ymo_oitrie_free(ymo_oitrie_t* oitrie);

/** Given an input string, return the string id. */
ymo_status_t ymo_oitrie_get_id(
        int* hdr_id,
        const ymo_oitrie_t* restrict oitrie,
        register const char* restrict str_in);

#endif /* YMO_TRIE_H */



