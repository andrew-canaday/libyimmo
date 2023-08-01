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




#ifndef YMO_MQTT_SESSION_H
#define YMO_MQTT_SESSION_H
#include "yimmo_config.h"
#include "yimmo.h"
#include "core/ymo_bucket.h"
#include "ymo_mqtt.h"

/** ymo_mqtt_session.h
 * ====================
 *
 */

/*---------------------------------------------------------------*
 * Types:
 *---------------------------------------------------------------*/
typedef uint8_t ymo_mqtt_fixed_hdr_t;

typedef struct ymo_mqtt_str {
    char*     val;
    uint16_t  len;
} ymo_mqtt_str_t;

#define YMO_MQTT_STR_FREE(mqtt_str) \
        if( mqtt_str.val ) { \
            YMO_FREE(mqtt_str.val); \
            mqtt_str.val = NULL;  \
            mqtt_str.len = 0; \
        }

YMO_ENUM8_TYPEDEF(mqtt_parse_state) {
    MQTT_PARSE_FIXED_CTRLPACK,
    MQTT_PARSE_FIXED_LENGTH,
    MQTT_PARSE_VARHDR_PAYLOAD,
    MQTT_PARSE_COMPLETE,
} YMO_ENUM8_AS(mqtt_parse_state_t);


YMO_ENUM8_TYPEDEF(ymo_mqtt_proto_version) {
    MQTT_PROTO_3_1_0,
    MQTT_PTOTO_3_1_1,
} YMO_ENUM8_AS(ymo_mqtt_proto_version_t);

typedef struct ymo_mqtt_msg {
    size_t  msg_len;
    size_t  payload_len;
    union {
        size_t  len_mult;
        size_t  msg_remain;
    };
    char*               var_hdr;
    char*               payload;
    mqtt_parse_state_t  parse_state;
    uint8_t             msg_type;
} ymo_mqtt_msg_t;


/** Internal structure used to manage a yimmo mqtt session.
 */
typedef struct ymo_mqtt_session {
    /** TODO: can we get rid of embedding the conn in the session? */
    ymo_conn_t*               conn;
    ymo_mqtt_msg_t            msg_in;
    ymo_mqtt_state_t          state;
    ymo_bucket_t*             send_head;
    ymo_bucket_t*             send_tail;
    char                      client_id[24];
    uint16_t                  ka_timer;
    uint8_t                   connect_flags;
    ymo_mqtt_str_t            username;
    ymo_mqtt_str_t            password;
    ymo_mqtt_str_t            will_topic;
    ymo_mqtt_str_t            will_msg;
    ymo_mqtt_proto_version_t  proto_id;
} ymo_mqtt_session_t;


/*---------------------------------------------------------------*
 *  Functions
 *---------------------------------------------------------------*/

/** Create a new MQTT session. */
ymo_mqtt_session_t* ymo_mqtt_session_create(ymo_conn_t* conn);

/** Clear and deallocate memory associated with the variable header and payload
 * sections of the incoming message structure on the given session.
 */
void ymo_mqtt_session_msg_free(ymo_mqtt_session_t* session);

/** Clear and free the given mqtt session object, including any nested data
 * which has been dynamically allocated. */
void ymo_mqtt_session_free(ymo_mqtt_session_t* session);

void ymo_mqtt_session_send(
        ymo_mqtt_session_t* session,
        uint8_t msg_type, uint8_t msg_flags,
        const char* buf, size_t len);

#endif /* YMO_MQTT_SESSION_H */



