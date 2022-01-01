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

/*======================================================================
 * !!! TODO:
 *
 * Recent hacking has made a pretty gross read out of a bunch of
 * this stuff!!
 *
 * Clean it up!
 *
 * (WIP!)
 *----------------------------------------------------------------------*/

#define YMO_HTRACE 0
#if defined(YMO_HTRACE) && YMO_HTRACE == 1
# define HTRACE(fmt, ...) \
    ymo_log_trace("\033[00;31;01;m"fmt "\033[00;m", __VA_ARGS__);
# define HTRACE_UUID(fmt, ...) ymo_log_trace_uuid(fmt, __VA_ARGS__);
#else
# define HTRACE(fmt, ...)
# define HTRACE_UUID(fmt, ...)
#endif /* YMO_HTRACE */

/*---------------------------------------------------------------*
 *  Declarations
 *---------------------------------------------------------------*/
/** Round pointer value up to next word boundary: */
#define PTR32_ALIGN (sizeof(uint32_t) - 1)
#define PTR32_FLOOR(p) (((uintptr_t)p) & ~PTR32_ALIGN)
#define PTR32_CEIL(p) ((((uintptr_t)p) + PTR32_ALIGN) & ~PTR32_ALIGN)

#define YMO_WS_FLAGS_RESERVED(f) \
    (f & (YMO_WS_FLAG_RSV1 | YMO_WS_FLAG_RSV2 | YMO_WS_FLAG_RSV3) )



/*---------------------------------------------------------------*
 *  Yimmo WS Parse Functions:
 *---------------------------------------------------------------*/
/* Parse websocket op code: */
ssize_t
ymo_ws_parse_op(ymo_ws_session_t* session, char* buffer, size_t len)
{
    session->frame_in.flags.packed = *buffer;

    /* Fail on reserved bits in frame: */
    if( YMO_WS_FLAGS_RESERVED(session->frame_in.flags.packed) ) {
        unsigned packed = session->frame_in.flags.packed;
        ymo_log_debug("Message uses reserved flag bits: 0x%x",
                packed);
        return YMO_ERROR_SSIZE_T(EBADMSG);
    }

    int fin_required = 0;
    switch( session->frame_in.flags.op_code ) {
        case YMO_WS_OP_CLOSE:
            fin_required = 1;
            break;

        case YMO_WS_OP_CONTINUATION:
            if( !session->msg_type ) {
                ymo_log_debug(
                        "Received unexpected continuation (state: 0x%x)",
                        (int)session->msg_type);
                return YMO_ERROR_SSIZE_T(EINVAL);
            }
            break;

        case YMO_WS_OP_TEXT:
        /* fallthrough */
        case YMO_WS_OP_BINARY:
            if( session->msg_type ) {
                ymo_log_debug(
                        "Expected continuation; received: 0x%x",
                        session->frame_in.flags.op_code);
                return YMO_ERROR_SSIZE_T(EINVAL);
            }

            if( !session->frame_in.flags.fin ) {
                session->msg_type = session->frame_in.flags.op_code;
            }
            break;

        case YMO_WS_OP_PING:
        /* fallthrough */
        case YMO_WS_OP_PONG:
            fin_required = 1;
            break;

        /* --- RESERVED: --- */
        default:
            ymo_log_debug("Received reserved op code (%i) from WS client.",
                    session->frame_in.flags.op_code);
            return YMO_ERROR_SSIZE_T(EBADMSG);
            break;
    }

    if( fin_required && !session->frame_in.flags.fin ) {
        ymo_log_debug("Got control code with no FIN bit set: 0x%0x",
                session->frame_in.flags.packed);
        return YMO_ERROR_SSIZE_T(EBADMSG);
    }

    session->frame_in.parse_state = WS_PARSE_LEN;
    return 1;
}


/* Parse websocket op code: */
ssize_t
ymo_ws_parse_len(ymo_ws_session_t* session, char* buffer, size_t len)
{
    char ch = *buffer;
    session->frame_in.masked = ch & YMO_WS_FLAG_MASKED;
    uint8_t msg_len = ch & YMO_WS_MASK_LEN;

    /* As per RFC6455: bail if client has not set masking flag. */
    if( !session->frame_in.masked ) {
        return YMO_ERROR_SSIZE_T(EBADMSG);
    }

    /* 0-125 ==> msg_len is the total payload length: */
    if( msg_len <= 125 ) {
        session->frame_in.len = msg_len;
        session->frame_in.parse_state = WS_PARSE_MASKING_KEY;

        ymo_status_t ok_alloc;
        if( (ok_alloc = ymo_ws_session_alloc_frame(session, msg_len)) ) {
            return YMO_ERROR_SSIZE_T(ok_alloc);
        }
        goto skip_op_check;
    }
    /* 126 ==> 2 bytes follow for length: */
    else if( msg_len == 126 ) {
        session->frame_in.len_idx = 2;
        session->frame_in.parse_state = WS_PARSE_LEN_EXTENDED;
    }
    /* 127 ==> 8 bytes follow for length: */
    else if( msg_len == 127 ) {
        session->frame_in.len_idx = 8;
        session->frame_in.parse_state = WS_PARSE_LEN_EXTENDED;
    }

    /* Control frames cannot have an extended length: */
    if( (session->frame_in.flags.packed & 0x08) ) {
        ymo_log_debug(
                "Got op 0x%x with extended length.",
                session->frame_in.flags.op_code);
        return YMO_ERROR_SSIZE_T(EBADMSG);
    }

skip_op_check:
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
            return YMO_ERROR_SSIZE_T(EBADMSG);
        }

        ymo_status_t ok_alloc;
        if( (ok_alloc = ymo_ws_session_alloc_frame(
                        session, session->frame_in.len)) ) {
            return YMO_ERROR_SSIZE_T(ok_alloc);
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
#if 0 /* Don't trace masking key anymore. */
            ymo_log_trace("Message length: %lu", session->frame_in.len);
            ymo_log_trace("Masking key: 0x%02x%02x%02x%02x",
                    session->frame_in.masking_key[0],
                    session->frame_in.masking_key[1],
                    session->frame_in.masking_key[2],
                    session->frame_in.masking_key[3]);
#endif

            /* The spec says the length *can* be zero, so...if the
             * frame header said the payload length is zero, we
             * can skip the payload parsing:
             */
            if( session->frame_in.len ) {
                session->frame_in.parse_state = WS_PARSE_PAYLOAD;
            } else {
                /* HACK: add a zero-length bucket so the callback is
                 * still invoked:
                 *
                 * (NOTE: are we sure recv_head is empty?)
                 */
                session->recv_tail = ymo_bucket_create(
                        session->recv_tail, NULL,
                        NULL, 0,
                        NULL, 0);

                if( !session->recv_head ) {
                    session->recv_head = session->recv_tail;
                }
                session->frame_in.parse_state = WS_PARSE_COMPLETE;
            }
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

    /* Book-keeping + append data to bucket chain: */
    char* buffer_in = session->frame_in.buffer + session->frame_in.parsed;
    session->frame_in.parsed += parse_len;


    /* Validate UTF-8
     *
     * TODO: First conception was to do this char-by-char as we loop through
     *       the buffer during unmask.
     *
     * - Give it a shot. Does it tank the readability? (is it tanked already?)
     * - Will we miss out on optimizations in the unmask loops if we
     *   introduce a bunch of conditionals?
     * - Measure it.
     *
     */
    int parse_done = (session->frame_in.flags.fin
                      && (session->frame_in.parsed == session->frame_in.len));

    if( (session->frame_in.flags.op_code == YMO_WS_OP_TEXT
         || session->msg_type == YMO_WS_OP_TEXT)
        && (session->frame_in.len
            && ymo_check_utf8(
                    &session->utf8_state, buffer, parse_len, parse_done
                    ) != YMO_UTF8_VALID) ) {

        ymo_log_debug("Encountered bad UTF-8 after parsing %zu bytes",
                session->frame_in.parsed + parse_len);
        return YMO_ERROR_SSIZE_T(EILSEQ);
    }

    /* HACK HACK: slap incoming data in a frame buffer: */
    if( session->frame_in.flags.op_code != YMO_WS_OP_PONG ) {
        memcpy(buffer_in, buffer, parse_len);

        /* TODO:
         * - bounds check
         * - we can skip the memcpy if we do the unmasking into the buffer
         * - allocate dynamically, rather than upper bound?
         */
    }

    if( session->frame_in.parsed >= session->frame_in.len ) {
        ymo_log_trace("msg parse complete (%lu)", session->frame_in.parsed);
        session->frame_in.parse_state = WS_PARSE_COMPLETE;

        if( session->frame_in.flags.op_code != YMO_WS_OP_PONG ) {
            session->recv_tail = ymo_bucket_create(
                    session->recv_tail, NULL,
                    NULL, 0,
                    session->frame_in.buffer,
                    session->frame_in.len);
        }

        if( !session->recv_head ) {
            session->recv_head = session->recv_tail;
        }

        /* Anticipate a new message if fin bit is set for NON-CONTROL frames: */
        switch( session->frame_in.flags.op_code ) {
            case YMO_WS_OP_CONTINUATION:
            /* fallthrough */
            case YMO_WS_OP_TEXT:
            /* fallthrough */
            case YMO_WS_OP_BINARY:
                if( session->frame_in.flags.fin ) {
                    session->msg_type = 0;
                }
                break;
            default:
                /* For control frames: don't update the message state. */
                break;
        }
    }
    return parse_len;
}

