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
 *  along with this program.  If not, see <ws://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

#include "ymo_config.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_util.h"
#include "ymo_ws_parse.h"

/*---------------------------------------------------------------*
 *  Declarations
 *---------------------------------------------------------------*/
/** Round pointer value up to next word boundary: */
#define PTR32_ALIGN (sizeof(uint32_t) - 1)
#define PTR32_FLOOR(p) (((uintptr_t)p) & ~PTR32_ALIGN)
#define PTR32_CEIL(p) ((((uintptr_t)p) + PTR32_ALIGN) & ~PTR32_ALIGN)


/*---------------------------------------------------------------*
 *  Yimmo WS Parse Functions:
 *---------------------------------------------------------------*/
/* Parse websocket op code: */
ssize_t
ymo_ws_parse_op(ymo_ws_session_t* session, char* buffer, size_t len)
{
    session->frame_in.flags.packed = *buffer;
    switch( session->frame_in.flags.op_code ) {
        /* --- WS_MSG_CONTINUATION: --- */
        case YMO_WS_OP_CONTINUATION:
            if( session->msg_state != WS_MSG_CONTINUE ) {
                ymo_log_debug(
                        "Received unexpected continuation (state: %i)",
                        session->msg_state);
                errno = EINVAL;
                return -1;
            }
            break;

        /* --- WS_MSG_START: --- */
        case YMO_WS_OP_TEXT:
        /* fallthrough */
        case YMO_WS_OP_BINARY:
        /* fallthrough */
        case YMO_WS_OP_CLOSE:
        /* fallthrough */
        case YMO_WS_OP_PING:
        /* fallthrough */
        case YMO_WS_OP_PONG:
            if( session->msg_state == WS_MSG_CONTINUE ) {
                ymo_log_debug(
                        "Expected continuation; received: 0x%x",
                        session->frame_in.flags.op_code);
                errno = EINVAL;
                return -1;
            }

            if( !session->frame_in.flags.fin ) {
                session->msg_state = WS_MSG_CONTINUE;
            }
            break;

        /* --- RESERVED: --- */
        default:
            ymo_log_debug("Received reserved op code (%i) from WS client.",
                    session->frame_in.flags.op_code);
            errno = EBADMSG;
            return -1;
    }
    session->frame_in.parse_state = WS_PARSE_LEN;
    return 1;
}

/* Parse websocket op code: */
ssize_t
ymo_ws_parse_len(ymo_ws_session_t* session, char* buffer, size_t len)
{
    char ch = *buffer++;
    session->frame_in.masked = ch & YMO_WS_FLAG_MASKED;
    uint8_t msg_len = ch & YMO_WS_MASK_LEN;

    /* As per RFC6455: bail if client has not set masking flag. */
    if( !session->frame_in.masked ) {
        errno = EBADMSG;
        return -1;
    }

    /* 0-125 ==> msg_len is the total payload length: */
    if( msg_len <= 125 ) {
        session->frame_in.len = msg_len;
        session->frame_in.parse_state = WS_PARSE_MASKING_KEY;
    }
    /* 126 ==> 2 bytes follow for length: */
    else if( msg_len == 126 ) {
        session->frame_in.len_idx = 2;
        session->frame_in.parse_state = WS_PARSE_LEN_EXTENDED;
    }
    /* 127 ==> 8 bytes follow for length: */
    else if( msg_len == 127 ) {
        session->frame_in.len_idx = 4;
        session->frame_in.parse_state = WS_PARSE_LEN_EXTENDED;
    }
    return 1;
}


/* Parse websocket op code: */
ssize_t
ymo_ws_parse_len_ext(ymo_ws_session_t* session, char* buffer, size_t len)
{
    uint8_t* start = (uint8_t*)buffer;
    uint8_t* current = start;
    while( len-- && session->frame_in.len_idx )
    {
        uint64_t val = (*current++);
        session->frame_in.len += val << (8 * --session->frame_in.len_idx);
    }

    if( session->frame_in.len_idx == 0 ) {
        /* Top bit must be unset: */
        if( session->frame_in.len & 0x8000000000000000 ) {
            errno = EBADMSG;
            return -1;
        }
        session->frame_in.parse_state = WS_PARSE_MASKING_KEY;
    }

    return (ssize_t)(current - start);
}


/* Parse websocket op code: */
ssize_t
ymo_ws_parse_masking_key(ymo_ws_session_t* session, char* buffer, size_t len)
{
    if( !session->frame_in.masked ) {
        session->frame_in.parse_state = WS_PARSE_PAYLOAD;
        return 0;
    }

    char* current = buffer;
    char* end = buffer + len;

    do {
        session->frame_in.masking_key[session->frame_in.mask_mod++] = *current++;

        if( session->frame_in.mask_mod == 4 ) {
            ymo_log_trace("Message length: %lu", session->frame_in.len);
            ymo_log_trace("Masking key: 0x%02x%02x%02x%02x",
                    session->frame_in.masking_key[0],
                    session->frame_in.masking_key[1],
                    session->frame_in.masking_key[2],
                    session->frame_in.masking_key[3]);
            session->frame_in.parse_state = WS_PARSE_PAYLOAD;
            goto masking_parse_done;
        }

    } while( current < end );

masking_parse_done:
    return current - buffer;
}

static uint32_t
get_mask32(ymo_ws_session_t* session)
{
    uint8_t mask_parts[4] __attribute__ ((aligned(sizeof(uint32_t))));
    for( int i = 0; i < 4; ++i )
    {
        mask_parts[i] =
            session->frame_in.masking_key[session->frame_in.mask_mod++ % 4];
    }
    return (uint32_t) *((uint32_t*)mask_parts);
}

/* Horrible type-punning convenience type. */
typedef union {
    char*     c;
    uint32_t* b4;
} buff_ptr_t;


/* Parse websocket op code: */
ssize_t
ymo_ws_parse_payload(ymo_ws_session_t* session, char* buffer, size_t len)
{
    size_t msg_remain = session->frame_in.len - session->frame_in.parsed;
    size_t parse_len = YMO_MIN(len, msg_remain);
    size_t parse_remain = parse_len;
    char* parse_end = buffer + parse_len;
    buff_ptr_t current = {.c = buffer};
    /* Unmask: */
    if( session->frame_in.masked ) {

        /* Do the initial round of bytes before the alignment boundary: */
        size_t end = YMO_MIN(parse_len, PTR32_CEIL(buffer)-(uintptr_t)buffer);
        while( end-- > 0 && parse_remain-- > 0 ) {
            char ch = *current.c;
            *current.c++ =
                ch ^ session->frame_in.masking_key[session->frame_in.mask_mod++ % 4];
        }

        /* Do the next round of bytes, 4 bytes at a time: */
        end = PTR32_FLOOR(parse_end)-(uintptr_t)current.c;
        uint32_t mask = get_mask32(session);
        while( end > 0 && parse_remain > 0 ) {
            uint32_t val = *current.b4;
            *(current.b4++) = val ^ mask;
            end -= 4;
            parse_remain -= 4;
        }

        /* Do the end round of bytes, post alignment boundary: */
        end = (uintptr_t)parse_end - (uintptr_t)current.c;
        while( end-- > 0 && parse_remain-- > 0 ) {
            char ch = *current.c;
            *current.c++ =
                ch ^ session->frame_in.masking_key[session->frame_in.mask_mod++ % 4];
        }
    }

    /* Book-keeping + append data to bucket chain: */
    session->frame_in.parsed += parse_len;
    session->recv_tail = ymo_bucket_create_cpy(
            session->recv_tail, NULL, buffer, parse_len);

    if( !session->recv_head ) {
        session->recv_head = session->recv_tail;
    }

    /* If this was a final message, mark it complete. */
    if( session->frame_in.parsed >= session->frame_in.len ) {
        ymo_log_trace("msg parse complete (%lu)", session->frame_in.parsed);
        session->frame_in.parse_state = WS_PARSE_COMPLETE;

        /* Anticipate a new message if fin bit is set: */
        if( session->frame_in.flags.fin ) {
            session->msg_state = WS_MSG_START;
        }
    }
    return parse_len;
}




