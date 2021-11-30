/*============================================================================*
 * Copyright (c) 2014 Andrew T. Canaday
 *
 * NOTICE: THIS EXAMPLE FILE IS LICENSED SEPARATELY FROM THE REST OF LIBYIMMO.
 *
 * This file is licensed under the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *----------------------------------------------------------------------------*/

/** # Example: MQTT Server
 *
 * **WARNING**: This is stale + MQTT is not WIP/high-priority, at the moment!
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



