/*=============================================================================
 * libyimmo: C MQTT/Websocket framework
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

#include "ymo_config.h"

#include "yimmo.h"
#include "ymo_alloc.h"
#include "ymo_conn.h"
#include "mqtt/ymo_mqtt_session.h"

ymo_mqtt_session_t* ymo_mqtt_session_create(ymo_conn_t* conn)
{
    ymo_mqtt_session_t* session = YMO_NEW0(ymo_mqtt_session_t);
    if( session ) {
        session->conn = conn;
    }
    return session;
}

void ymo_mqtt_session_msg_free(ymo_mqtt_session_t* session)
{
    if( session->msg_in.var_hdr ) {
        YMO_FREE(session->msg_in.var_hdr);
    }
    session->msg_in.msg_len = 0;
    session->msg_in.var_hdr = session->msg_in.payload = NULL;
    return;
}

void ymo_mqtt_session_free(ymo_mqtt_session_t* session)
{
    ymo_bucket_free_all(session->send_head);
    ymo_mqtt_session_msg_free(session);
    YMO_MQTT_STR_FREE(session->username);
    YMO_MQTT_STR_FREE(session->password);
    YMO_MQTT_STR_FREE(session->will_topic);
    YMO_MQTT_STR_FREE(session->will_msg);
    YMO_DELETE(ymo_mqtt_session_t, session);
}


/** Encode a size_t into an MQTT "remaining length" field.
 *
 * :param out_buf:  buffer into which encoded value is written
 * :param buf_len:  output variable indicating number of bytes of buff used
 * :param len:  length to be encoded
 * :returns: :c:macro:`YMO_OKAY` on success; ERANGE on error
 */
static inline ssize_t encode_remain(char* out_buf, size_t len)
{
    ssize_t buf_len = 0;

    do {
        uint8_t e = len % 0x80;
        len = len >> 7; /* Won't the compiler do this anyway if we "/0x80"? */
        if( len ) {
            e |= 0x80;
        }

        *out_buf++ = e;
        ++buf_len;
    } while( len );

    return buf_len;
}


void ymo_mqtt_session_send(
        ymo_mqtt_session_t* session,
        uint8_t msg_type, uint8_t msg_flags,
        const char* buf, size_t len)
{
    size_t fixed_len = 1;
    char fixed_hdr[5];

    /* TODO: *optional* message type validation (by config) here. */
    fixed_hdr[0] = (msg_type & YMO_MQTT_FIXED_HDR_TYPE_MASK) |
                   (msg_flags & YMO_MQTT_FIXED_HDR_FLAG_MASK);

    fixed_len += encode_remain(fixed_hdr+1, len);

    ymo_bucket_t* fixed_bucket = ymo_bucket_create_cpy(
            session->send_tail, NULL, fixed_hdr, fixed_len);
    ymo_bucket_t* varhdr_payload = ymo_bucket_create(
            fixed_bucket, NULL, NULL, 0, buf, len);

    if( !session->send_head ) {
        session->send_head = fixed_bucket;
    }
    session->send_tail = varhdr_payload;
    ymo_conn_tx_enable(session->conn, 1);
    return;
}



