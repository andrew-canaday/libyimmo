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


#include "yimmo_config.h"

#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <yaml.h>

#include "yimmo.h"

#include "ymo_log.h"
#include "ymo_alloc.h"
#include "ymo_list.h"
#include "ymo_yaml.h"

/* This, like the others, mostly exists for development. */
#ifndef YMO_YAML_TRACE
#  define YMO_YAML_TRACE 0
#endif

#if defined(YMO_YAML_TRACE) && YMO_YAML_TRACE == 1
# define YAML_TRACE(fmt, ...) ymo_log_trace(fmt, __VA_ARGS__);
#else
# define YAML_TRACE(fmt, ...)
#endif /* YMO_YAML_TRACE */

#define YMO_YAML_PATH_MAX_LEN 1023
#define YMO_YAML_DOC_MAX_SIZE 8192


/** TODO: all of this would be way less gnarly with a finite state machine. */

/*---------------------------------------------------------------------------*
 * Types:
 *---------------------------------------------------------------------------*/

/** Type used to store nodes in a yaml document. */
struct ymo_yaml_node {
    YMO_LIST_HEAD();
    ymo_yaml_type_t       y_type;

    const char*           value;
    const char*           terminal; /* TODO: introduced for debugging purposes only. */
    struct ymo_yaml_node* parent;
    struct ymo_yaml_node* child;
};


/** This is the top-level descriptor for a yaml document. */
struct ymo_yaml_doc {
    char*            f_path;
    size_t           len;
    char*            buffer;
    ymo_yaml_node_t* root;

    /* Only used during parsing: */
    size_t  remain;
    char*   end;
};


/*---------------------------------------------------------------------------*
 * Static Prototypes:
 *---------------------------------------------------------------------------*/

static ymo_yaml_node_t* ymo_yaml_node_create(
        ymo_yaml_doc_t* doc, ymo_yaml_node_t* parent, yaml_event_t* event);
static ymo_status_t ymo_yaml_parse(FILE* in_file, ymo_yaml_doc_t* doc);
static inline void print_indent(FILE* out_file, int16_t indent);
static void ymo_yaml_print_node(FILE* out_file, ymo_yaml_node_t* node, int16_t indent);
static ymo_yaml_doc_t* ymo_yaml_doc_create(const char* f_path, size_t f_size);
static void ymo_yaml_node_free(ymo_yaml_node_t* node);


/*---------------------------------------------------------------------------*
 * Doc Functions:
 *---------------------------------------------------------------------------*/

ymo_yaml_doc_t* ymo_yaml_load_file(const char* f_path)
{
    ymo_yaml_doc_t* doc = NULL;
    errno = 0;

    FILE* yaml_file = fopen(f_path, "r");
    if( !yaml_file ) {
        goto yaml_load_err;
    }

    fseek(yaml_file, 0, SEEK_END);
    long f_size = ftell(yaml_file);

    if( f_size < 0 ) {
        goto yaml_load_file_err;
    }

    doc = ymo_yaml_doc_create(f_path, f_size);
    if( !doc ) {
        goto yaml_load_file_err;
    }

    rewind(yaml_file);
    if( ymo_yaml_parse(yaml_file, doc) != YMO_OKAY ) {
        goto yaml_load_file_err;
    }

    return doc;

yaml_load_file_err:
    fclose(yaml_file);
    ymo_yaml_doc_free(doc);

yaml_load_err:
    ymo_log_warning("YAML load failed: %s", strerror(errno));
    return NULL;
}


const char* ymo_yaml_doc_filepath(const ymo_yaml_doc_t* doc)
{
    const char* r_val = NULL;
    if( doc ) {
        return r_val = doc->f_path;
    }
    return r_val;
}


ymo_yaml_node_t* ymo_yaml_doc_root(const ymo_yaml_doc_t* doc)
{
    ymo_yaml_node_t* r_val = NULL;
    if( doc ) {
        r_val = doc->root;
    }
    return r_val;
}


void ymo_yaml_doc_free(ymo_yaml_doc_t* doc)
{
    if( !doc ) {
        return;
    }

    if( doc->f_path ) {
        YMO_FREE(doc->f_path);
    }

    if( doc->root ) {
        ymo_yaml_node_free(doc->root);
    }

    if( doc->buffer ) {
        YMO_FREE(doc->buffer);
    }

    YMO_DELETE(ymo_yaml_doct_t, doc);
    return;
}


void ymo_yaml_doc_print_json_hack(FILE* out_file, const ymo_yaml_doc_t* doc)
{
    ymo_yaml_print_node(out_file, doc->root, -2);
    return;
}


/*---------------------------------------------------------------------------*
 * Node Functions:
 *---------------------------------------------------------------------------*/

ymo_yaml_type_t ymo_yaml_node_type(const ymo_yaml_node_t* node)
{
    if( node ) {
        return node->y_type;
    }
    return YMO_YAML_ERROR;
}


const ymo_yaml_node_t* ymo_yaml_item_next(const ymo_yaml_node_t* list, const ymo_yaml_node_t* current)
{
    const ymo_yaml_node_t* node = NULL;
    if( current ) {
        node = YMO_LIST_NEXT(current, const ymo_yaml_node_t);
    } else if( list && list->child ) {
        node = list->child;
    }
    return node;
}


const ymo_yaml_node_t* ymo_yaml_object_get(const ymo_yaml_node_t* node, const char* key)
{
    const ymo_yaml_node_t* v_node = NULL;
    if( node ) {
        if( node->y_type != YMO_YAML_OBJECT ) {
            errno = EINVAL;
            goto yaml_object_get_done;
        }

        const ymo_yaml_node_t* k_node = ymo_yaml_item_next(node, NULL);
        while( k_node ) {
            if( k_node->y_type == YMO_YAML_SCALAR ) {
                if( k_node->value ) {
                    /* TODO: something more secure than strcasecmp... */
                    if( !strcasecmp(k_node->value, key) ) {
                        v_node = k_node->child;
                        break;
                    }
                }
            } else {
                /* If a mapping has a non-scalar key, that's an issue too. */
                errno = EINVAL;
                return NULL;
            }

            k_node = ymo_yaml_item_next(node, k_node);
        }
    }

yaml_object_get_done:
    return v_node;
}


const ymo_yaml_node_t* ymo_yaml_key_value(const ymo_yaml_node_t* key)
{
    const ymo_yaml_node_t* v_node = NULL;
    if( key && key->y_type == YMO_YAML_SCALAR && key->child ) {
        v_node = key->child;
    }
    return v_node;
}


const char* ymo_yaml_node_as_str(const ymo_yaml_node_t* node)
{
    if( node ) {
        return node->value;
    }

    return NULL;
}


int ymo_yaml_node_as_long(const ymo_yaml_node_t* node, long* value)
{
    if( !node || !node->value ) {
        return EINVAL;
    }

    char* endptr;
    long yaml_l = strtol(node->value, &endptr, 0);
    if( *endptr == '\0' ) {
        *value = yaml_l;
        return 0;
    }

    return EINVAL;
}


int ymo_yaml_node_as_float(const ymo_yaml_node_t* node, float* value)
{
    if( !node || !node->value ) {
        return EINVAL;
    }

    char* endptr;
    float yaml_f = strtof(node->value, &endptr);
    if( *endptr == '\0' ) {
        *value = yaml_f;
        return 0;
    }

    return EINVAL;
}


/*-------------------------------------------*
 * Module static functions:
 *-------------------------------------------*/

static ymo_yaml_node_t* ymo_yaml_node_create(
        ymo_yaml_doc_t* doc, ymo_yaml_node_t* parent, yaml_event_t* event)
{
    errno = 0;
    ymo_yaml_node_t* node = YMO_NEW0(ymo_yaml_node_t);
    if( !node ) {
        errno = ENOMEM;
        return NULL;
    }

    ymo_yaml_node_t* r_node = node;
    node->parent = parent;

    size_t buf_len = 0;
    char* end = doc->end;
    switch( event->type ) {
        case YAML_SCALAR_EVENT:
            /* A scalar should always have a parent node. */
            if( !parent ) {
                errno = EINVAL;
                goto yaml_parse_node_bail;
            }

            buf_len = event->data.scalar.length+1;
            if( buf_len < doc->remain ) {
                node->y_type = YMO_YAML_SCALAR;
                node->value = end;
                snprintf(end, buf_len, "%s", event->data.scalar.value);
                end += buf_len+1;
                doc->remain -= buf_len;

                /* HACK HACK HACK */
                switch( parent->y_type ) {
                    /* Just silence compiler warnings here... */
                    case YMO_YAML_ERROR:
                        errno = EINVAL;
                        goto yaml_parse_node_bail;
                        break;

                    /* Scalar with parent type of scalar is the VALUE of a
                     * key: value pair.
                     */
                    case YMO_YAML_SCALAR:
                        node->terminal = "";
                        r_node = parent;
                        while( r_node && r_node->y_type == YMO_YAML_SCALAR ) {
                            /* The key is the direct parent. We want to pop
                             * back up to the object (the parent's parent).
                             */
                            r_node = r_node->parent;
                        }
                        break;

                    /* Scalar with a parent type of sequence is just an item
                     * in the sequence.
                     */
                    case YMO_YAML_SEQUENCE:
                        node->terminal = "";
                        r_node = parent;
                        break;

                    /* Scalar with a parent type of object is the key in a
                     * key:value pair.
                     */
                    case YMO_YAML_OBJECT:
                        node->terminal = ":";
                        break;
                }
            } else {
                errno = EFBIG;
                goto yaml_parse_node_bail;
            }
            break;

        case YAML_MAPPING_START_EVENT:
            YAML_TRACE("YAML: %s", "YAML_MAPPING_START_EVENT");
            node->y_type = YMO_YAML_OBJECT;
            node->value = "{";
            node->terminal = "}";
            break;
        case YAML_SEQUENCE_START_EVENT:
            YAML_TRACE("YAML: %s", "YAML_SEQUENCE_START_EVENT");
            node->y_type = YMO_YAML_SEQUENCE;
            node->value = "[";
            node->terminal = "]";
            break;
        default:
            errno = EINVAL;
            goto yaml_parse_node_bail;
            break;
    }

    if( parent ) {
        if( !parent->child ) {
            parent->child = node;
        } else {
            ymo_yaml_node_t* head = parent->child;
            ymo_yaml_node_t* last = YMO_LIST_LAST(head, ymo_yaml_node_t);
            YMO_LIST_APPEND(last, node);
        }
    }

    doc->end = end;
    if( !doc->root ) {
        doc->root = r_node;
    }

#if defined(YMO_YAML_TRACE) && YMO_YAML_TRACE == 1
    YAML_TRACE("Created %s", node->value);
    if( parent ) {
        YAML_TRACE(" - with parent %s", parent->value);
    }
    if( node->child ) {
        YAML_TRACE(" - with child %s", node->child->value);
    }
    YAML_TRACE("  returning: %s", r_node->value);
#endif /* YMO_YAML_TRACE */
    return r_node;

yaml_parse_node_bail:
    YMO_DELETE(ymo_yaml_node_t, node);
    return NULL;
}


static void ymo_yaml_node_free(ymo_yaml_node_t* node)
{
yaml_node_free:
    if( !node ) {
        return;
    }

    ymo_yaml_node_free(node->child);
    ymo_yaml_node_t* next = YMO_LIST_NEXT(node, ymo_yaml_node_t);
    YMO_FREE(node);

    node = next;
    goto yaml_node_free;
    return;
}


static ymo_status_t ymo_yaml_parse(FILE* in_file, ymo_yaml_doc_t* doc)
{
    ymo_status_t r_val = YMO_OKAY;
    ymo_yaml_node_t* parent = NULL;

    yaml_parser_t parser;
    yaml_event_t event;

    int done = 0;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, in_file);

    /* Read the event sequence. */
    while( !done ) {

        /* Get the next event. */
        if( !yaml_parser_parse(&parser, &event)) {
            r_val = EINVAL;
            goto yaml_parse_done;
        }

        switch( event.type ) {
            /** An empty event. */
            case YAML_NO_EVENT:
                YAML_TRACE("YAML: %s", "YAML_NO_EVENT");
                break;

            case YAML_STREAM_START_EVENT:
                YAML_TRACE("YAML: %s", "YAML_STREAM_START_EVENT");
                break;
            case YAML_STREAM_END_EVENT:
                YAML_TRACE("YAML: %s", "YAML_STREAM_END_EVENT");
                break;

            case YAML_DOCUMENT_START_EVENT:
                YAML_TRACE("YAML: %s", "YAML_DOCUMENT_START_EVENT");
                break;
            case YAML_DOCUMENT_END_EVENT:
                YAML_TRACE("YAML: %s", "YAML_DOCUMENT_END_EVENT");
                break;

            case YAML_ALIAS_EVENT:
                YAML_TRACE("YAML: %s", "YAML_ALIAS_EVENT");
                break;

            /* Data: */
            case YAML_SCALAR_EVENT:
            /* fallthrough */
            case YAML_SEQUENCE_START_EVENT:
            /* fallthrough */
            case YAML_MAPPING_START_EVENT:
                if( (parent = ymo_yaml_node_create(doc, parent, &event)) == NULL ) {
                    r_val = errno;
                    done = 1;
                }
                break;
            case YAML_SEQUENCE_END_EVENT:
            case YAML_MAPPING_END_EVENT:
pop_parent:
#if defined(YMO_YAML_TRACE) && YMO_YAML_TRACE == 1
                if( parent ) {
                    YAML_TRACE("Popping parent: %s", parent->value);
                    if( parent->child ) {
                        YAML_TRACE("  - with child: %s", parent->child->value);
                    }
                    if( parent->parent ) {
                        YAML_TRACE("  - to: %s", parent->parent->value);
                    }
                }
#endif /* YMO_YAML_TRACE */
                parent = parent->parent;
                if( parent && parent->y_type == YMO_YAML_SCALAR ) {
                    goto pop_parent;
                }
                break;
        }

        done = done || (event.type == YAML_STREAM_END_EVENT);
        yaml_event_delete(&event);
    }

yaml_parse_done:
    /* TODO: also delete the event, if we bailed out. */
    yaml_parser_delete(&parser);
    return r_val;
}


static inline void print_indent(FILE* out_file, int16_t indent)
{
    for( uint8_t i = 0; i < indent; i++ ) {
        fprintf(out_file, "%c", ' ');
    }
}


static void ymo_yaml_print_node(FILE* out_file, ymo_yaml_node_t* node, int16_t indent)
{
    indent = 0;
    if( !node ) {
        return;
    }

    print_indent(out_file, indent);

    if( node->y_type == YMO_YAML_SCALAR ) {
        fprintf(out_file, "\"%s\"", node->value);
        fprintf(out_file, "%s", node->terminal);
    } else {
        fprintf(out_file, "%s", node->value);
    }

    if( node->child ) {
        print_indent(out_file, indent);
    }

    ymo_yaml_print_node(out_file, node->child, indent+4);
    if( node->y_type != YMO_YAML_SCALAR ) {
        fprintf(out_file, "%s", node->terminal);
    }
    ymo_yaml_node_t* next = YMO_LIST_NEXT(node, ymo_yaml_node_t);
    if( next ) {
        fprintf(out_file, "%s", ", ");
        print_indent(out_file, indent);
        ymo_yaml_print_node(out_file, next, indent);
        print_indent(out_file, indent);
    }
    return;
}


static ymo_yaml_doc_t* ymo_yaml_doc_create(const char* f_path, size_t f_size)
{
    ymo_yaml_doc_t* doc = YMO_NEW0(ymo_yaml_doc_t);
    if( !doc ) {
        errno = ENOMEM;
        goto doc_create_bail;
    }

    /* ...folks, I know...
     * Hang in, okay?
     */
    size_t path_len = strnlen(f_path, YMO_YAML_PATH_MAX_LEN);
    doc->f_path = YMO_ALLOC(path_len+1);
    if( !doc->f_path ) {
        errno = ENOMEM;
        goto doc_create_bail;
    }

    strncpy(doc->f_path, f_path, path_len);
    doc->f_path[path_len] = '\0';

    doc->remain = doc->len = f_size + 1;
    if( doc->len > YMO_YAML_DOC_MAX_SIZE ) {
        errno = EFBIG;
        goto doc_create_bail;
    }

    /* Technically, the doc could be bigger than this, due to anchors, etc.
     * We'll keep an eye on it: */
    doc->buffer = YMO_ALLOC(doc->len);
    if( !doc->buffer ) {
        errno = ENOMEM;
        goto doc_create_bail;
    }
    doc->buffer[f_size] = '\0';
    doc->end = doc->buffer;

    return doc;

doc_create_bail:
    ymo_yaml_doc_free(doc);
    return NULL;
}

