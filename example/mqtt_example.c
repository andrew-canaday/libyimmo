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


/** # Example: MQTT Server
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ev.h>
#include "yimmo.h"
#include "ymo_log.h"
#include "ymo_mqtt.h"

/* Default TCP accept waitlist length. */
#define DEFAULT_LISTEN_BACKLOG 100

/* Default port to listen on for mqtt connections. */
#define DEFAULT_MQTT_PORT 11883

/* MQTT server initialization */
static ymo_server_t* init_mqtt(struct ev_loop* loop, in_port_t port);

/* Stop the ev loop on sigint: */
static void sigint_handler(struct ev_loop* loop, ev_signal* w, int revents);

/** ## Step 1: Create the protocol/server objects.
 *
 */
static ymo_server_t* init_mqtt(struct ev_loop* loop, in_port_t port)
{
    int n = 0;
    ymo_proto_t* mqtt_proto = NULL;
    ymo_server_t* mqtt_srv = NULL;
    ymo_server_config_t mqtt_cfg;

    /* Initialize echo mqtt_srv params: */
    memset(&mqtt_cfg, 0, sizeof(mqtt_cfg));
    mqtt_cfg.loop = loop;
    mqtt_proto = ymo_proto_mqtt_create();
    if( !mqtt_proto ) {
        goto mqtt_bail;
    }

    mqtt_cfg.port = port;
    mqtt_cfg.flags = (YMO_SERVER_REUSE_ADDR | YMO_SERVER_REUSE_PORT);
    mqtt_cfg.listen_backlog = DEFAULT_LISTEN_BACKLOG;

    /* Set up the yimmo mqtt_srv: */
    mqtt_srv = ymo_server_create(&mqtt_cfg, mqtt_proto);
    if( mqtt_srv ) {
        if( (n = ymo_server_init(mqtt_srv)) ) {
            ymo_log(YMO_LOG_ERROR, strerror(n));
            ymo_server_free(mqtt_srv);
            mqtt_srv = NULL;
        }
    }
    return mqtt_srv;

mqtt_bail:
    return NULL;
}


/** ## Step 2: Start it up!
 *
 */
int main(int argc, char** argv)
{
    in_port_t mqtt_port = DEFAULT_MQTT_PORT;
    ymo_server_t* mqtt_srv = NULL;

    /* libev loop init: */
#ifdef USE_KQUEUE
    struct ev_loop* loop = ev_default_loop(EVBACKEND_KQUEUE);
#else
    struct ev_loop* loop = ev_default_loop(0);
#endif /* USE_KQUEUE */

    /* Create the MQTT server: */
    mqtt_srv = init_mqtt(loop, mqtt_port);

    /* Install a sigint handler so we can terminate gracefully: */
    ev_signal sigint_watcher;
    ev_signal_init(&sigint_watcher, sigint_handler, SIGINT);
    ev_signal_start(loop, &sigint_watcher);

    /* Run both! */
    if( mqtt_srv ) {
        ev_run(loop,0);
        ymo_log(YMO_LOG_INFO, "Shutting down!");
        ymo_server_free(mqtt_srv);
    } else {
        ymo_log(YMO_LOG_ERROR, "Server failed to start!");
    }
    return 0;
}


static void sigint_handler(struct ev_loop* loop, ev_signal* w, int revents)
{
    ev_break(loop, EVBREAK_ALL);
    return;
}



