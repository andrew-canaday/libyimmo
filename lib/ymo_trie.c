/*=============================================================================
 *
 *  Copyright (c) 2014 Andrew Canaday
 *
 *  This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "yimmo.h"
#include "ymo_alloc.h"
#include "ymo_log.h"
#include "ymo_util.h"
#include "ymo_trie.h"

ymo_trie_t* ymo_trie_create(void)
{
    ymo_trie_t* trie = YMO_NEW0(ymo_trie_t);
    if( trie ) {
        trie->root = ymo_trie_node_create();

        /* Bail if we can't ccreate the root node: */
        if( !trie->root ) {
            ymo_trie_free(trie);
            return NULL;
        }
    }
    return trie;
}

void ymo_trie_free(ymo_trie_t* trie)
{
    if( trie->root ) {
        ymo_trie_node_free(trie->root);
        trie->root = NULL;
    }
    YMO_DELETE(ymo_trie_t, trie);
    return;
}

ymo_status_t ymo_trie_add_string(
        ymo_trie_t* trie, const char* str_in)
{
    ymo_trie_node_t* current = trie->root;
    unsigned char c;
    while( (c = ymo_tolower(*(str_in++))) != '\0' )
    {
        if( current->children[c] == NULL ) {
            ymo_trie_node_t* node = ymo_trie_node_create();
            if( node ) {
                ++trie->no_nodes;
                current->children[c] = node;
            } else {
                return ENOMEM;
            }
        }
        current = current->children[c];
    }
    ++trie->no_values;
    current->value = 1;
    return 0;
}

ymo_trie_node_t* ymo_trie_node_create(void)
{
    return YMO_NEW0(ymo_trie_node_t);
}

void ymo_trie_node_free(ymo_trie_node_t* node)
{
    int i;
    for( i = 0; i < YMO_TRIE_NO_CHILDREN; ++i )
    {
        if( node->children[i] ) {
            ymo_trie_node_free(node->children[i]);
        }
    }
    YMO_DELETE(ymo_trie_node_t, node);
    return;
}

static void trie_node_annotate(
        ymo_trie_t* trie,
        ymo_trie_node_t* node,
        size_t* node_id,
        size_t* value_id
        )
{
    int i;
    for( i = 0; i < YMO_TRIE_NO_CHILDREN; ++i )
    {
        if( node->children[i] ) {
            node->children[i]->index = (*node_id)++;
            if( node->children[i]->value ) {
                node->children[i]->value_id = (*value_id)++;
            }
            trie_node_annotate(trie, node->children[i], node_id, value_id);
        }
    }
}

static ymo_status_t oitrie_node_create(
        ymo_oitrie_node_t* oitrie, ymo_trie_node_t* node, size_t* max_offset)
{
    ymo_status_t status = YMO_OKAY;
    int i = 0;
    int t = 0;
    while( i < YMO_TRIE_NO_CHILDREN )
    {
        ymo_trie_node_t* left = NULL;
        ymo_trie_node_t* right = NULL;

        /* Find a "left" node; continue until found: */
        left = node->children[i++];
        if( !left ) {
            continue;
        }

        /* Store the char value for this node, based on index: */
        oitrie[left->index].value = i-1;

        /* Determine if the "left" node has a child and mark it: */
        for( t = 0; t < YMO_TRIE_NO_CHILDREN; ++t )
        {
            if( left->children[t] ) {
                oitrie[left->index].flags |= OITRIE_HAS_CHILD;
                break;
            }
        }

        /* If the "left" node has a value make a note of it, and store it: */
        if( left->value ) {
            oitrie[left->index].flags |= OITRIE_IS_TERMINAL;
            oitrie[left->index].hdr_id = left->value_id;
        }

        /* Search for a "right node": */
        t = i;
        while( t < YMO_TRIE_NO_CHILDREN )
        {
            right = node->children[t++];
            /* If we found one, store the offset between "left" and "right"
             * in the left node, and break: */
            if( right ) {
                size_t offset = right->index - left->index;
                if( offset > *max_offset ) {
                    *max_offset = offset;
                }

                if( offset > YMO_OITRIE_MAX_OFFSET ) {
                    ymo_log_warning(
                            "Header table offset (%lu) exceeds max (%i)",
                            offset, YMO_OITRIE_MAX_OFFSET);
                    return EOVERFLOW;
                }
                oitrie[left->index].offset = right->index - left->index;
                break;
            }
        }

        status = oitrie_node_create(oitrie, left, max_offset);
        if( status != YMO_OKAY ) {
            return status;
        }
        i = --t;
    }
    return status;
}

ymo_oitrie_t* ymo_oitrie_create(ymo_trie_t* trie)
{
    ymo_log_trace(
            "Creating compressed trie with %lu nodes.", trie->no_nodes);

    /* Allocate space enough for a list of compressed nodes: */
    ymo_oitrie_t* oitrie = YMO_NEW0(ymo_oitrie_t);
    if( !oitrie ) {
        return oitrie;
    }

    ymo_oitrie_node_t* root = (ymo_oitrie_node_t*)calloc(
            trie->no_nodes,sizeof(ymo_oitrie_node_t));

    if( root ) {
        size_t node_id = 0;
        size_t value_id = 0;
        size_t max_offset = 0;
        oitrie->root = root;
        oitrie->no_nodes = trie->no_nodes;
        oitrie->no_values = trie->no_values;

        /* Annotate the existing trie: */
        trie_node_annotate(trie, trie->root, &node_id, &value_id);

        /* Fill in the flattened compressed trie: */
        oitrie_node_create(root, trie->root, &max_offset);
        ymo_log_trace("Compressed trie max offset: %lu", max_offset);
    } else {
        YMO_DELETE(ymo_oitrie_t, oitrie);
        oitrie = NULL;
    }
    return oitrie;
}

size_t ymo_oitrie_sizeof(const ymo_oitrie_t* oitrie)
{
    size_t total_bytes = sizeof(ymo_oitrie_t);
    total_bytes += sizeof(ymo_oitrie_node_t) * oitrie->no_nodes;
    return total_bytes;
}

void ymo_oitrie_free(ymo_oitrie_t* oitrie)
{
    if( oitrie->no_nodes ) {
        YMO_DELETE(ymo_oitrie_node_t, oitrie->root);
    }
    YMO_DELETE(ymo_oitrie_t, oitrie);
    return;
}

ymo_status_t ymo_oitrie_get_id(
        int* hdr_id,
        const ymo_oitrie_t* restrict oitrie,
        register const char* restrict str_in)
{
    register unsigned char c = ymo_tolower(*(str_in++));
    ymo_oitrie_node_t* current = oitrie->root;
    for( ;; )
    {
        /* Test to see if this node is a match: */
        if( c == current->value ) {
            c = ymo_tolower(*(str_in++) );

            /* If we're at end of string *and* have a value, we found a match:
             */
            if( c == '\0' && (current->flags & OITRIE_IS_TERMINAL) ) {
                (*hdr_id) = current->hdr_id;
                return 0;
            }

            if( current->flags & OITRIE_HAS_CHILD ) {
                /* Step one oitrie node ahead in the list if we have a child: */
                ++current;
            } else {
                /* Otherwise, we've reached the end of this trie path: bail! */
                break;
            }
        }
        /* Otherwise, see if there is another at this level we can jump to: */
        else if( current->offset ) {
            /* Jump by the offset in the current "left" node: */
            current += (current->offset);
        }
        /* Otherwise, we're out of luck: bail! */
        else {
            break;
        }
    }

    return EINVAL;
}



