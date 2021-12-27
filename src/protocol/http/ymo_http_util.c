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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *===========================================================================*/

#include "ymo_config.h"
#include "core/ymo_server.h"
#include "ymo_http.h"
#include "ymo_log.h"

/** Default TCP accept waitlist length. */
#ifndef HTTP_DEFAULT_LISTEN_BACKLOG
#define HTTP_DEFAULT_LISTEN_BACKLOG 512
#endif /* HTTP_DEFAULT_LISTEN_BACKLOG */

ymo_server_t* ymo_http_simple_init(
        struct ev_loop* loop,
        in_port_t port,
        ymo_http_cb_t http_callback,
        ymo_http_upgrade_handler_t** upgrade_handlers,
        void* data
        )
{

    /* Default ev_loop: */
    if( loop == NULL ) {
        loop = ev_default_loop(0);
    }

    /* Default HTTP config: */
    ymo_server_config_t http_cfg;
    memset(&http_cfg, 0, sizeof(http_cfg));
    http_cfg.loop = loop;
    http_cfg.port = port;
    http_cfg.flags = (YMO_SERVER_REUSE_ADDR | YMO_SERVER_REUSE_PORT);
    http_cfg.listen_backlog = HTTP_DEFAULT_LISTEN_BACKLOG;

    int n = 0;
    ymo_proto_t* http_proto = NULL;
    ymo_server_t* http_srv = NULL;

    /* Initialize echo http_srv params: */
    http_proto = ymo_proto_http_create(
            NULL,
            http_callback,
            NULL,
            NULL,
            NULL,
            data
            );
    if( !http_proto ) {
        goto http_bail;
    }

    if( upgrade_handlers != NULL ) {
        while( *upgrade_handlers != NULL ) {
            ymo_http_add_upgrade_handler(http_proto, *upgrade_handlers);
            ++upgrade_handlers;
        }
    }

    /* Set up the yimmo http_srv: */
    http_srv = ymo_server_create(&http_cfg, http_proto);
    if( http_srv ) {
        if( (n = ymo_server_init(http_srv)) ) {
            ymo_log(YMO_LOG_ERROR, strerror(n));
            ymo_server_free(http_srv);
            http_srv = NULL;
        }
    }
    return http_srv;

http_bail:
    return NULL;
}

