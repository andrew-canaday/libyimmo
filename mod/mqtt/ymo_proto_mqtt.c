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
 *.
 *
 *===========================================================================*/

#include "ymo_config.h"

#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_alloc.h"
#include "ymo_net.h"
#include "ymo_proto.h"

#include "ymo_conn.h"
#include "ymo_proto_mqtt.h"
#include "ymo_mqtt_parse.h"

/*---------------------------------------------------------------*
 *  Yimmo MQTT Protocol:
 *---------------------------------------------------------------*/
static ymo_proto_vt_t ymo_default_mqtt_proto = {
    .init_cb = &ymo_proto_mqtt_init,
    .cleanup_cb = &ymo_proto_mqtt_cleanup,
    .conn_init_cb = &ymo_proto_mqtt_conn_init,
    .read_cb = &ymo_proto_mqtt_read,
    .write_cb = &ymo_proto_mqtt_write,
    .conn_cleanup_cb = &ymo_proto_mqtt_conn_cleanup,
};

typedef struct ymo_mqtt_proto_data {
    void* data;
} ymo_mqtt_proto_data_t;


ymo_proto_t* ymo_proto_mqtt_create(void)
{
    /* Allocate and initialize the protocol object: */
    ymo_proto_t* mqtt_proto = YMO_NEW0(ymo_proto_t);
    if( !mqtt_proto ) {
        goto proto_bail;
    }
    mqtt_proto->name = "MQTT";
    mqtt_proto->vtable = ymo_default_mqtt_proto;

    /* Allocate the nested private data: */
    ymo_mqtt_proto_data_t* data = YMO_NEW(ymo_mqtt_proto_data_t);
    if( !data ) {
        goto proto_free_and_bail;
    }
    mqtt_proto->data = (void*)data;

    return mqtt_proto;

proto_free_and_bail:
    YMO_DELETE(ymo_proto_t, mqtt_proto);
proto_bail:
    return NULL;
}


/*---------------------------------------------------------------*
 *  Yimmo MQTT Protocol Init Callback:
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_mqtt_init(ymo_proto_t* proto, ymo_server_t* server)
{
    return YMO_OKAY;
}


/*---------------------------------------------------------------*
 *  Yimmo MQTT Protocol Cleanup Callback:
 *---------------------------------------------------------------*/
void ymo_proto_mqtt_cleanup(ymo_proto_t* proto, ymo_server_t* server)
{
    return;
}


/*---------------------------------------------------------------*
 *  Yimmo MQTT Protocol Client Init Callback:
 *---------------------------------------------------------------*/
void* ymo_proto_mqtt_conn_init(void* proto_data, ymo_conn_t* conn)
{
    ymo_mqtt_session_t* session = NULL;
    session = ymo_mqtt_session_create(conn);
    return (void*)session;
}


/*---------------------------------------------------------------*
 *  Yimmo MQTT Protocol Client Cleanup Callback:
 *---------------------------------------------------------------*/
void ymo_proto_mqtt_conn_cleanup(
        void* proto_data, ymo_conn_t* conn, void* conn_data)
{
    ymo_mqtt_session_t* session = conn_data;
    if( session ) {
        ymo_mqtt_session_free(session);
    }
    return;
}


/*---------------------------------------------------------------*
 *  Yimmo MQTT Protocol Read Callback:
 *---------------------------------------------------------------*/
const char connack_msg[] = {0x00, 0x00};

/* MQTT Protocol read callback: */
ssize_t ymo_proto_mqtt_read(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        char* recv_buf,
        size_t len)
{
    ymo_mqtt_session_t* session = conn_data;
    char* parse_buf = recv_buf;
    do {
        ssize_t n = 0;
        switch( session->msg_in.parse_state ) {
            case MQTT_PARSE_FIXED_CTRLPACK:
                n = ymo_mqtt_parse_fixed_ctrlpack(session, parse_buf, len);
                break;
            case MQTT_PARSE_FIXED_LENGTH:
                n = ymo_mqtt_parsed_fixed_len(session, parse_buf, len);
                break;
            case MQTT_PARSE_VARHDR_PAYLOAD:
                n = ymo_mqtt_parse_varhdr_payload(session, parse_buf, len);
                break;
            case MQTT_PARSE_COMPLETE:
                goto mqtt_parse_complete;
                break;
        }

        if( n >= 0 ) {
            ymo_log_trace("%i bytes parsed (remain=%lu)", (int)n, len);
            parse_buf += n;
            len -= n;
        } else {
            /* Errno has been set by the last parse function: */
            return -1;
        }
    } while(len);

    if( session->msg_in.parse_state == MQTT_PARSE_COMPLETE ) {
mqtt_parse_complete:
        switch( session->msg_in.msg_type ) {
            case YMO_MQTT_CONNECT:
                /* TODO: Check for client id collisions! */
                session->state = YMO_MQTT_STATE_AWAITING_AUTHENTICATION;

                /* TODO: authentication
                 * - Provide callback passed in to ymo_mqtt_proto_data_t
                 * - Flags for synchronous vs asychronous
                 */
                ymo_mqtt_session_send(
                        session, YMO_MQTT_CONNACK, 0, connack_msg, 2);
                session->state = YMO_MQTT_STATE_CONNECTED;
                break;

            default:
                /* TODO: I didn't leave a verbose note here, so I just know there is an item "to do"... */
                break;
        }
        mqtt_parse_reset(session);
    }
    return (parse_buf - recv_buf);
}


/*---------------------------------------------------------------*
 *  Yimmo MQTT Protocol Write Callback:
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_mqtt_write(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        int socket)
{
    ymo_status_t status = YMO_OKAY;
    ymo_mqtt_session_t* session = conn_data;

    if( session->send_head ) {
        status = ymo_send_buckets(
                socket, &session->send_head);
    }

    /* This means they're all sent: */
    if( !session->send_head ) {
        session->send_tail = NULL;
        ymo_conn_tx_enable(conn, 0);
    }
    return status;
}



