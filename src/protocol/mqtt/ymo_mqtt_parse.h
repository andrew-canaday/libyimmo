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



#ifndef YMO_MQTT_PARSE_H
#define YMO_MQTT_PARSE_H
#include "yimmo_config.h"
#include "yimmo.h"
#include "ymo_mqtt.h"
#include "ymo_mqtt_session.h"

/** ymo_mqtt_parse.h
 * ==================
 *
 */

/*---------------------------------------------------------------*
 *  Flags
 *---------------------------------------------------------------*/
/** http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718038: */
#define PUBLISH_RETAIN     0x01
#define PUBLISH_QOS1       0x02
#define PUBLISH_QOS2       0x04
#define PUBLISH_DUP        0x08

/** http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718022 */
#define PUBREL_FLAGS       0x02
#define SUBSCRIBE_FLAGS    0x02
#define UNSUBSCRIBE_FLAGS  0x02

/** http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718023 */
#define LEN_CONTINUATION 0x80
#define LEN_VALUE        0x7f
#define LEN_MAX_MULT     0x200000 /* (128*128*128) */

/** http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030 */
#define CONNECT_CLEAN_SESSION 0x02
#define CONNECT_WILL_FLAG     0x04
#define CONNECT_WILL_QOS      0x18
#define CONNECT_WILL_RETAIN   0x20
#define CONNECT_PASSWORD      0x40
#define CONNECT_USERNAME      0x80

/*---------------------------------------------------------------*
 *  Functions
 *---------------------------------------------------------------*/
/** Used to reset the parse state for this current session (e.g. after an
 * incoming message has been parsed to completion and handled at the protocol
 * level, this function is invoked to prepare for the next message). */
void mqtt_parse_reset(ymo_mqtt_session_t* session);

/** Parse control packet component of the fixed header: */
ssize_t ymo_mqtt_parse_fixed_ctrlpack(
        ymo_mqtt_session_t* session, const char* restrict buffer, size_t len);

/** Parse the remaining length component of the fixed header: */
ssize_t ymo_mqtt_parsed_fixed_len(
        ymo_mqtt_session_t* session, const char* buffer, size_t len);

/** Parse the variable header and payload: */
ssize_t ymo_mqtt_parse_varhdr_payload(
        ymo_mqtt_session_t* session, const char* restrict buffer, size_t len);

#endif /* YMO_MQTT_PARSE_H */



