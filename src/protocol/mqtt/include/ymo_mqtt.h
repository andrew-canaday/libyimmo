/*=============================================================================
 *
 * Copyright (c) 2014 Andrew Canaday
 *
 * This file is part of libyimmo (sometimes referred to as "yimmo" or "ymo").
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
 *
 *===========================================================================*/



#ifndef YMO_MQTT_H
#define YMO_MQTT_H
#include "yimmo.h"

/** MQTT API
 * ==========
 *
 * Yimmo MQTT public API.
 *
 * .. toctree::
 *   :hidden:
 *   :maxdepth: 3
 *
 *   index
 *
 * .. contents:: Contents
 *   :local:
 *
 */

/**---------------------------------------------------------------
 * Flags
 *---------------------------------------------------------------*/

/** http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020
 */
#define YMO_MQTT_FIXED_HDR_TYPE_MASK 0xF0

/** http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020
 */
#define YMO_MQTT_FIXED_HDR_FLAG_MASK 0x0F


/**---------------------------------------------------------------
 * Types
 *---------------------------------------------------------------*/

/**  */
YMO_ENUM8_TYPEDEF(ymo_mqtt_state) {
    YMO_MQTT_STATE_CONNECTING,
    YMO_MQTT_STATE_AWAITING_AUTHENTICATION,
    YMO_MQTT_STATE_CONNECTED,
    YMO_MQTT_STATE_DISCONNECTING,
    YMO_MQTT_STATE_DISCONNECTED
} YMO_ENUM8_AS(ymo_mqtt_state_t);


/** MQTT Control Packet Type
 * ..........................
 *
 * See http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718021
 * for more info.
 *
 */

/**  */
#define YMO_MQTT_CONNECT        0x10

/**  */
#define YMO_MQTT_CONNACK        0x20

/**  */
#define YMO_MQTT_PUBLISH        0x30

/**  */
#define YMO_MQTT_PUBACK         0x40

/**  */
#define YMO_MQTT_PUBREC         0x50

/**  */
#define YMO_MQTT_PUBREL         0x60

/**  */
#define YMO_MQTT_PUBCOMP        0x70

/**  */
#define YMO_MQTT_SUBSCRIBE      0x80

/**  */
#define YMO_MQTT_SUBACK         0x90

/**  */
#define YMO_MQTT_UNSUBSCRIBE    0xA0

/**  */
#define YMO_MQTT_UNSUBACK       0xB0

/**  */
#define YMO_MQTT_PINGREQ        0xC0

/**  */
#define YMO_MQTT_PINGRESP       0xD0

/**  */
#define YMO_MQTT_DISCONNECT     0xE0


/**---------------------------------------------------------------
 * MQTT Protocol Management
 *---------------------------------------------------------------*/


/**  */
ymo_proto_t* ymo_proto_mqtt_create(void);


#endif /* YMO_MQTT_H */

