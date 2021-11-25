# Example: MQTT Server


## Step 1: Create the protocol/server objects.



```C
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
```

## Step 2: Start it up!



```C
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
```

