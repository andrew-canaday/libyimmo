/*=============================================================================
 * libyimmo: C MQTT/WebSocket framework
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
 *.
 *
 *===========================================================================*/

#include "ymo_config.h"

#include <string.h>

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_alloc.h"
#include "ymo_util.h"
#include "ymo_mqtt_session.h"
#include "ymo_mqtt_parse.h"

/*---------------------------------------------------------------*
 * Misc:
 *---------------------------------------------------------------*/
const char* CTRL_PACK_NAMES[] = {
    "CONNECT",
    "CONNACK",
    "PUBLISH",
    "PUBACK",
    "PUBREC",
    "PUBREL",
    "PUBCOMP",
    "SUBSCRIBE",
    "SUBACK",
    "UNSUBSCRIBE",
    "UNSUBACK",
    "PINGREQ",
    "PINGRESP",
    "DISCONNECT",
};


/*---------------------------------------------------------------*
 * Convenience Macros:
 *---------------------------------------------------------------*/

/* Given a control packet type, get the associated name. */
#define CTRL_PACK_NAME(msg_type) \
    CTRL_PACK_NAMES[ (msg_type>>4) - 1]

/* Consume a 16-bit unsigned int from the char buffer "src", incrementing
 * the pointer as a side effect. */
#define PARSE_CONSUME_UINT16(dst, src) \
    dst = ((uint16_t)(*src++)) << 8; \
    dst += (uint16_t)(*src++)

/* Consume an 8-bit unsigned int from the char buffer "src", incrementing
 * the pointer as a side effect. */
#define PARSE_CONSUME_UINT8(dst, src) \
    dst = (uint8_t)(*src++)

/* Consume the string of length "len" from the char buffer "src", incrementing
 * the pointer as a side effect. */
#define PARSE_CONSUME_STRVAL(dst, src, len) \
    memcpy(dst, src, len); \
    src += len

/* CAUTION: hacky + no mem checking.
 * Initialize a ymo_mqtt_str_t object from data in a message buffer:
 * - Get the length of the next string
 * - Allocate enough space to house the new string
 * - Copy from the message buffer into the newly allocated space
 * - Increment the "src" pointer as a side effect.
 */
#define PARSE_STRDUP(dst, src) \
    PARSE_CONSUME_UINT16(dst.len, src); \
    dst.val = YMO_ALLOC(dst.len); \
    PARSE_CONSUME_STRVAL(dst.val, src, dst.len)


/*---------------------------------------------------------------*
 * Parse Utility Functions:
 *---------------------------------------------------------------*/
/* Transition: MQTT_PARSE_COMPLETE -> MQTT_PARSE_FIXED_CTRLPACK
 * Reset only those fields in the session which are used to parse
 * incoming messages. */
void mqtt_parse_reset(ymo_mqtt_session_t* session)
{
    session->msg_in.parse_state = MQTT_PARSE_FIXED_CTRLPACK;
    session->msg_in.msg_remain = 0;
    session->msg_in.msg_type = 0;
    ymo_mqtt_session_msg_free(session);
    return;
}


/* Transition: MQTT_PARSE_FIXED_CTRLPACK -> MQTT_PARSE_FIXED_LENGTH
 * Reset session fields used in remaining length calculations. */
static inline void mqtt_parse_len_init(ymo_mqtt_session_t* session)
{
    session->msg_in.len_mult = 1;
    ++session->msg_in.parse_state;
    return;
}


/* Transition: MQTT_PARSE_FIXED_LENGTH -> MQTT_PARSE_VARHDR_PAYLOAD
 * Reset session fields used in variable header parsing. */
static inline ymo_status_t mqtt_parse_varhdr_init(ymo_mqtt_session_t* session)
{
    session->msg_in.msg_remain = session->msg_in.msg_len;
    session->msg_in.var_hdr = YMO_ALLOC(session->msg_in.msg_len);
    if( !session->msg_in.var_hdr ) {
        return ENOMEM;
    }
    session->msg_in.payload = session->msg_in.var_hdr;
    ++session->msg_in.parse_state;
    return YMO_OKAY;
}


/* Transition: MQTT_PARSE_VARHDR_PAYLOAD -> MQTT_PARSE_COMPLETE */
static inline void mqtt_parse_complete(ymo_mqtt_session_t* session)
{
    session->msg_in.parse_state = MQTT_PARSE_COMPLETE;
    return;
}


/* Validate the variable header fields. */
static inline ymo_status_t mqtt_parse_varhdr(ymo_mqtt_session_t* session)
{
    session->msg_in.payload = session->msg_in.var_hdr;
    switch( session->msg_in.msg_type ) {
        case YMO_MQTT_CONNECT:
        {
            /* Protocol Identifiers/Level:
             * v3.1.0: "MQIsdp" / 0x03
             * v3.1.1: "MQTT" / 0x04
             */
            uint16_t proto_len;
            uint8_t proto_id;
            char proto_name[12];     /* TODO: Use vararray for proto_name? */
            PARSE_CONSUME_UINT16(proto_len, session->msg_in.payload);
            PARSE_CONSUME_STRVAL(proto_name, session->msg_in.payload, proto_len);
            PARSE_CONSUME_UINT8(proto_id, session->msg_in.payload);

            if( (proto_id == 0x04 && proto_len == 0x04 &&
                 !memcmp(proto_name, "MQTT", proto_len)) ||
                (proto_id == 0x03 && proto_len == 0x06 &&
                 !memcmp(proto_name, "MQIsdp", proto_len)) ) {

                session->proto_id = proto_id;

                /* Connect flags (1), Keep alive timer (2): */
                PARSE_CONSUME_UINT8(
                        session->connect_flags, session->msg_in.payload);
                PARSE_CONSUME_UINT16(
                        session->ka_timer, session->msg_in.payload);
            } else {
                goto bad_var_hdr;
            }
        }
        break;
        case YMO_MQTT_CONNACK:
            break;
        /* The following have a packet identifier: */
        case YMO_MQTT_PUBLISH:
        /* (Packet identifier only present on PUBLISH for QOS > 0) */
        case YMO_MQTT_PUBACK:
        case YMO_MQTT_PUBREC:
        case YMO_MQTT_PUBREL:
        case YMO_MQTT_PUBCOMP:
        case YMO_MQTT_SUBSCRIBE:
        case YMO_MQTT_SUBACK:
        case YMO_MQTT_UNSUBSCRIBE:
        case YMO_MQTT_UNSUBACK:
            break;
        case YMO_MQTT_PINGREQ:
        case YMO_MQTT_PINGRESP:
        case YMO_MQTT_DISCONNECT:
            break;
    }

    /* Payload length is total message length minus var hdr length: */
    session->msg_in.payload_len =
        session->msg_in.msg_len -
        (session->msg_in.payload - session->msg_in.var_hdr);
    return YMO_OKAY;

bad_var_hdr:
    ymo_log_trace("Received invalid variable header for \"%s\"",
            CTRL_PACK_NAME(session->msg_in.msg_type));
    return EINVAL;
}


/* Parse the payload: */
static inline ymo_status_t mqtt_parse_payload(ymo_mqtt_session_t* session)
{
    const char* current = session->msg_in.payload;
    switch( session->msg_in.msg_type ) {
        case YMO_MQTT_CONNECT:
        {
            /* Client identifier is always present */
            uint16_t cid_len;
            PARSE_CONSUME_UINT16(cid_len, current);
            PARSE_CONSUME_STRVAL(session->client_id, current, cid_len);
            ymo_log_trace("Got connect from client id \"%s\"",
                    session->client_id);

            /* Convenience macro used to copy special fields out of the
             * payload. */
                #define PARSE_CONNECT_FIELD(dst, flag) \
    if( session->connect_flags & flag ) { \
        PARSE_STRDUP(session->dst, current); \
        ymo_log_trace("%s: %.*s", #flag, \
                    session->dst.len, session->dst.val); \
    }

            PARSE_CONNECT_FIELD(will_topic, CONNECT_WILL_FLAG);
            PARSE_CONNECT_FIELD(will_msg, CONNECT_WILL_FLAG);
            PARSE_CONNECT_FIELD(username, CONNECT_USERNAME);
            PARSE_CONNECT_FIELD(password, CONNECT_PASSWORD);
        }
        break;
        case YMO_MQTT_CONNACK:
            break;
        /* The following have a packet identifier: */
        case YMO_MQTT_PUBLISH:
        /* (Packet identifier only present on PUBLISH for QOS > 0) */
        case YMO_MQTT_PUBACK:
        case YMO_MQTT_PUBREC:
        case YMO_MQTT_PUBREL:
        case YMO_MQTT_PUBCOMP:
        case YMO_MQTT_SUBSCRIBE:
        case YMO_MQTT_SUBACK:
        case YMO_MQTT_UNSUBSCRIBE:
        case YMO_MQTT_UNSUBACK:
            break;
        case YMO_MQTT_PINGREQ:
        case YMO_MQTT_PINGRESP:
        case YMO_MQTT_DISCONNECT:
            break;
    }

    return YMO_OKAY;

// bad_payload:
    return EINVAL;
}


/*---------------------------------------------------------------*
 * MQTT Parse Functions:
 *---------------------------------------------------------------*/
/* Parse control packet component of the fixed header: */
ssize_t ymo_mqtt_parse_fixed_ctrlpack(
        ymo_mqtt_session_t* session, const char* restrict buffer, size_t len)
{
    /* Convenience macro to keep our switch tidy: */
    #define ASSERT_CTRL_FLAG(flag_val) \
    if( msg_flags != flag_val ) { goto ctrl_inval; };

    /* Probably, this is the start of the message, but we check in case we
     * previously received only one byte: */
    ymo_mqtt_fixed_hdr_t fixed_hdr = (ymo_mqtt_fixed_hdr_t)*buffer;
    uint8_t msg_type = fixed_hdr & YMO_MQTT_FIXED_HDR_TYPE_MASK;
    uint8_t msg_flags = fixed_hdr & YMO_MQTT_FIXED_HDR_FLAG_MASK;

    switch( msg_type ) {
        case YMO_MQTT_CONNECT:
            if( session->state != YMO_MQTT_STATE_CONNECTING ) {
                goto ctrl_inval;
            }
            ASSERT_CTRL_FLAG(0);
            break;
        /* fallthrough: */
        case YMO_MQTT_CONNACK:
            ASSERT_CTRL_FLAG(0);
            break;
        /* The following have a packet identifier: */
        case YMO_MQTT_PUBLISH:
            /* (Packet identifier only present on PUBLISH for QOS > 0) */
            break;
        case YMO_MQTT_PUBACK:
        case YMO_MQTT_PUBREC:
            ASSERT_CTRL_FLAG(0);
            break;
        case YMO_MQTT_PUBREL:
            ASSERT_CTRL_FLAG(2);
            break;
        case YMO_MQTT_PUBCOMP:
            ASSERT_CTRL_FLAG(0);
            break;
        case YMO_MQTT_SUBSCRIBE:
            ASSERT_CTRL_FLAG(2);
            break;
        case YMO_MQTT_SUBACK:
            ASSERT_CTRL_FLAG(0);
            break;
        case YMO_MQTT_UNSUBSCRIBE:
            ASSERT_CTRL_FLAG(2);
            break;
        case YMO_MQTT_UNSUBACK:
        case YMO_MQTT_PINGREQ:
        case YMO_MQTT_PINGRESP:
            ASSERT_CTRL_FLAG(0);
            break;
        case YMO_MQTT_DISCONNECT:
            break;
    }

    /* All is okay: */
    session->msg_in.msg_type = msg_type;
    mqtt_parse_len_init(session);
    ymo_log_trace("%s received; Flags: 0x%x",
            CTRL_PACK_NAME(msg_type), (int)msg_flags);
    return 1;

ctrl_inval:
    ymo_log_trace("Invalid control packet: %u", (unsigned)fixed_hdr);
    return -1;
}


/* Parse the remaining length component of the fixed header. */
ssize_t ymo_mqtt_parsed_fixed_len(
        ymo_mqtt_session_t* session, const char* buffer, size_t len)
{
    size_t r_len = 0;
    const char* current = buffer;
    do {
        if( session->msg_in.len_mult > LEN_MAX_MULT ) { /* 128^3 */
            errno = EINVAL;
            return -1;
        }

        r_len = (size_t)(*current++);
        session->msg_in.msg_len +=
            (r_len & LEN_VALUE) * session->msg_in.len_mult;
        session->msg_in.len_mult *= 0x80;
    } while( len && (r_len & LEN_CONTINUATION) );

    if( !(r_len & LEN_CONTINUATION) ) {
        int n;
        ymo_log_trace("Message length: %lu", session->msg_in.msg_len);
        if( (n = mqtt_parse_varhdr_init(session)) ) {
            errno = n;
            return -1;
        }
    }
    return (current - buffer);
}


/* Parse the variable header: */
ssize_t ymo_mqtt_parse_varhdr_payload(
        ymo_mqtt_session_t* session, const char* restrict buffer, size_t len)
{
    size_t to_copy = YMO_MIN(len, session->msg_in.msg_remain);

    /* Cheat here and use the payload pointer to keep track of the end of
     * the buffer: */
    memcpy(session->msg_in.payload, buffer, to_copy);
    session->msg_in.msg_remain -= to_copy;
    session->msg_in.payload += to_copy;

    /* If we've received the entire var hdr and payload, process them: */
    if( session->msg_in.msg_remain == 0 ) {
        int n = 0;

        /* Parse headers: */
        if( (n = mqtt_parse_varhdr(session)) != YMO_OKAY ) {
            errno = n;
            return -1;
        }

        /* Parse body: */
        if( (n = mqtt_parse_payload(session)) == YMO_OKAY ) {
            // mqtt_parse_reset(session);
            mqtt_parse_complete(session);
        } else {
            errno = n;
            return -1;
        }

    }
    return to_copy;
}

