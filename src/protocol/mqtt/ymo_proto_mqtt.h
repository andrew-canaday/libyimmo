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




#ifndef YMO_MQTT_PROTO_MQTT_H
#define YMO_MQTT_PROTO_MQTT_H
#include "ymo_config.h"
#include "yimmo.h"
#include "ymo_mqtt.h"

/** ymo_proto_mqtt.h
 * ==================
 *
 */

/*---------------------------------------------------------------*
 *  Functions
 *---------------------------------------------------------------*/
ymo_status_t ymo_proto_mqtt_init(
        ymo_proto_t* proto, ymo_server_t* server);
void ymo_proto_mqtt_cleanup(
        ymo_proto_t* proto, ymo_server_t* server);
void* ymo_proto_mqtt_conn_init(
        void* proto_data, ymo_conn_t* conn);
void ymo_proto_mqtt_conn_cleanup(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data);

/** MQTT Parse main entry point.
 * :param server: ymo_server issuing the parse request
 * :param session: MQTT session we're parsing
 * :param proto_data: MQTT-specific data created by proto init
 * :param recv_buf: server read buffer
 * :param len: number of bytes in recv_buf
 *
 * Returns number of bytes parsed on success, -1 on failure.
 */
ssize_t ymo_proto_mqtt_read(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        char* recv_buf,
        size_t len);

/** MQTT protocol write callback */
ymo_status_t ymo_proto_mqtt_write(
        void* proto_data,
        ymo_conn_t* conn,
        void* conn_data,
        int socket);

#endif /* YMO_MQTT_PROTO_MQTT_H */



