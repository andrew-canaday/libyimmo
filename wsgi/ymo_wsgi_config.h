/*=============================================================================
 *
 *  Copyright (c) 2021 Andrew Canaday
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

/******************************************************************************
 * __          __             _____    _   _   _____   _   _    _____         *
 * \ \        / /     /\     |  __ \  | \ | | |_   _| | \ | |  / ____|  _     *
 *  \ \  /\  / /     /  \    | |__) | |  \| |   | |   |  \| | | |  __  (_)    *
 *   \ \/  \/ /     / /\ \   |  _  /  | . ` |   | |   | . ` | | | |_ |        *
 *    \  /\  /     / ____ \  | | \ \  | |\  |  _| |_  | |\  | | |__| |  _     *
 *     \/  \/     /_/    \_\ |_|  \_\ |_| \_| |_____| |_| \_|  \_____| (_)    *
 *                                                                            *
 *                                                                            *
 * THIS IS A TOTAL KLUDGE!!                                                   *
 *                                                                            *
 * Don't judge. I was tired of setting up env vars and mashed some WIP        *
 * branches together...I'll tidy it.                                          *
 *                                                                            *
 *   _____  ____  _____  _______     ___ _                                    *
 *  / ____|/ __ \|  __ \|  __ \ \   / / | |                                   *
 * | (___ | |  | | |__) | |__) \ \_/ /| | |                                   *
 *  \___ \| |  | |  _  /|  _  / \   / | | |                                   *
 *  ____) | |__| | | \ \| | \ \  | |  |_|_|                                   *
 * |_____/ \____/|_|  \_\_|  \_\ |_|  (_|_)                                   *
 *                                                                            *
 ******************************************************************************/


#ifndef YMO_WSGI_CONFIG
#define YMO_WSGI_CONFIG

typedef struct {
    ymo_wsgi_proc_t* w_proc;
    int              argc;
    char* const*     argv;
} ymo_wsgi_config_t;

void usage(FILE* usage_out, char* bin_name);
static void ymo_wsgi_config(ymo_wsgi_config_t* w_config);
static const char* config_from_optarg(ymo_wsgi_config_t* w_config, char yopt);
static void config_from_yaml(ymo_wsgi_config_t* w_config, const char* f_path);
static int get_module_app(ymo_wsgi_proc_t* w_proc, int argc, char* const* argv);


static struct option longopts[] = {
    { "help",        no_argument,       NULL, 'h' },
    { "log-level",   required_argument, NULL, 'l' }, /* YIMMO_LOG_LEVEL */
    { "config",      required_argument, NULL, 'c' }, /* YIMMO_WSGI_CONFIG */
    { "port",        required_argument, NULL, 'P' }, /* YIMMO_WSGI_PORT */
    { "no-proc",     required_argument, NULL, 'p' }, /* YIMMO_WSGI_NO_PROC */
    { "no-threads",  required_argument, NULL, 't' }, /* YIMMO_WSGI_NO_THREADS */
    { NULL,     0,                      NULL,  0  }
};


void usage(FILE* usage_out, char* bin_name)
{
    fprintf(usage_out, "\n\n%s v%i.%i.%i\n", basename(bin_name),
            YIMMO_VERSION_MAJOR,
            YIMMO_VERSION_MINOR,
            YIMMO_VERSION_PATCH);
    fprintf(usage_out, "Usage: %s [OPTIONS] WSGI_MODULE:WSGI_APP\n", basename(bin_name));

    fputs("\nOptions:\n", usage_out);

    fputs("  --config, -c     : Path to the yimmo-wsgi config yaml\n", usage_out);
    fputs("  --log-level, -l  : Yimmo log level\n", usage_out);
    fputs("  --port, -P       : Port\n", usage_out);
    fputs("  --no-proc, -p    : Number of processes to run\n", usage_out);
    fputs("  --no-threads, -t : Number of worker threads per proc\n", usage_out);
    fputs("  --help, -h       : Display usage info (this)\n", usage_out);

    fputs("\nEnvironment Variables:\n", usage_out);

    fputs("  YIMMO_LOG_LEVEL           : libyimmo log level\n", usage_out);
    fputs("  YIMMO_SERVER_IDLE_TIMEOUT : socket-level idle disconnect timeout\n", usage_out);
    fputs("  YIMMO_WSGI_CONFIG         : yimmo wsgi config file path\n", usage_out);
    fputs("  YIMMO_WSGI_NO_PROC        : number of yimmo WSGI processes to run\n", usage_out);
    fputs("  YIMMO_WSGI_NO_THREADS     : number of worker threads per process\n", usage_out);
    fputs("  YIMMO_WSGI_MODULE         : WSGI module (if not provided as arg)\n", usage_out);
    fputs("  YIMMO_WSGI_APP            : WSGI app (if not provided as arg)\n", usage_out);
    fputs("  YIMMO_TLS_CERT_PATH       : TLS certificate path\n", usage_out);
    fputs("  YIMMO_TLS_KEY_PATH        : TLS private key path\n", usage_out);

    fputs("\nConfiguration precedence (greatest to least):\n", usage_out);
    fputs("  - command line parameters\n", usage_out);
    fputs("  - environment variables\n", usage_out);
    fputs("  - config files\n", usage_out);

    fputs(
            "\n"
            "# Config Example:\n"
            "---\n"
            "log_level: INFO\n"
            "wsgi:\n"
            "  port: 8081\n"
            "  tls:\n"
            "    cert: /path/to/site.crt\n"
            "    key: /path/to/cert.pem\n"
            "  no_proc: 2\n"
            "  no_threads: 2\n"
            , stderr);
    return;
}


/* HACK HACK HACK.
 *
 * TODO: may as well either make a combo parser with precedence or else
 *       run each config_from_* once each, storing results in a struct
 *       that gets merged.
 */
static const char* ymo_config_get_value(
        ymo_wsgi_config_t* w_config,
        char yopt,
        const char* env_name,
        ...)
{
    va_list args;
    va_start(args, env_name);

    const char* yval = NULL;
    const char* val = NULL;

    /* First check file: */
    if( (val = ymo_yaml_node_as_str(ymo_yaml_doc_vget(w_config->w_proc->cfg, args))) ) {
        yval = val;
    }
    va_end(args);

    /* Then check env: */
    if( (val = getenv(env_name)) ) {
        yval = val;
    }

    /* Then check command line: */
    if( (val = config_from_optarg(w_config, yopt)) ) {
        yval = val;
    }

    return yval;
}


static void ymo_wsgi_config(ymo_wsgi_config_t* w_config)
{
    char* endptr;
    long l_param;
    const char* val = NULL;

    /* First, see if we have a config file from env or cli and load it: */
    if( (val = ymo_config_get_value(w_config, 'c', "YIMMO_WSGI_CONFIG", NULL)) ) {
        config_from_yaml(w_config, val);
    }

    /* Log level: */
    if( (val = ymo_config_get_value(w_config, 'l', "YIMMO_LOG_LEVEL", "log_level", NULL)) ) {
        ymo_log_set_level_by_name(val);
    }

    /* Port: */
    if( (val = ymo_config_get_value(w_config, 'P', "YIMMO_WSGI_PORT", "wsgi", "port", NULL)) ) {
        l_param = strtol(val, &endptr, 0);
        if( *endptr == '\0' ) {
            w_config->w_proc->port = l_param;
        } else {
            fprintf(stderr, "Invalid value for port: \"%s\"\n", val);
            exit(-1);
        }
    } else {
        w_config->w_proc->port = DEFAULT_HTTP_PORT;
    }

    /* Number of processes: */
    if( (val = ymo_config_get_value(w_config, 'p', "YIMMO_WSGI_NO_PROC", "wsgi", "no_proc", NULL)) ) {
        l_param = strtol(val, &endptr, 0);
        if( *endptr == '\0' ) {
            w_config->w_proc->no_wsgi_proc = l_param;
        } else {
            fprintf(stderr, "Invalid value for no_proc: \"%s\"\n", val);
            exit(-1);
        }
    }

    /* Number of threads: */
    if( (val = ymo_config_get_value(w_config, 'w', "YIMMO_WSGI_NO_THREADS", "wsgi", "no_threads", NULL)) ) {
        l_param = strtol(val, &endptr, 0);
        if( *endptr == '\0' ) {
            w_config->w_proc->no_wsgi_threads = atoi(val); /* TODO: no atoi! */
        } else {
            fprintf(stderr, "Invalid value for no_threads: \"%s\"\n", val);
            exit(-1);
        }
    }

    /* Module and app can only come from cli (already parsed, if so) or env: */
    if( !w_config->w_proc->module ) {
        w_config->w_proc->module = getenv("YIMMO_WSGI_MODULE");
    }

    if( !w_config->w_proc->app ) {
        w_config->w_proc->app = getenv("YIMMO_WSGI_APP");
    }


    /* TODO: TLS handled in ymo_wsgi_server.c */
    return;
}


/** Yimmo WSGI Config file format
 * ------------------------------
 *
 * Example:
 *     ----
 *     log_level: INFO
 *     wsgi:
 *       port: 8081
 *       no_proc: 1
 *       no_threads: 1
 *       tls:
 *         cert: /path/to/site.crt
 *         key: /path/to/cert.pem
 *
 * TODO:
 *  - YMO_LOG_LEVEL_MAX        Compile-time log-level max                                   NOTICE
 *  - YMO_LOG_LEVEL_DEFAULT    Compile-time log-level default.                              WARNING
 *  - YMO_SERVER_IDLE_TIMEOUT  Default connection idle-disconnect timeout.                  5
 *  - YMO_HTTP_RECV_BUF_SIZE   maximum number of bytes allocated for headers, per-request.  1024
 *  - YMO_HTTP_SEND_BUF_SIZE   maximum number of bytes allocated for headers, per-response. 1024
 *  - YMO_SERVER_RECV_BUF_SIZE the server read buffer.                                      8192
 *  - YMO_HTTP_REQ_WS_SIZE     maximum WebSocket message chunk size.                        1024
 *  - YMO_MQTT_RECV_BUF_SIZE   maximum MQTT received payload size                           4096
 *  - YMO_BUCKET_MAX_IOVEC     maximum iovec array length for sendmsg()                     32
 *
 */
static void config_from_yaml(ymo_wsgi_config_t* w_config, const char* f_path)
{
    if( !f_path ) {
        return;
    }

    ymo_yaml_doc_t* doc = ymo_yaml_load_file(f_path);
    if( !doc ) {
        fprintf(stderr, "\nERROR: bad config file: %s\n", f_path);
        exit(-1);
    }

    const ymo_yaml_node_t* root = ymo_yaml_doc_root(doc);
    if( !root ) {
        goto yaml_config_err;
    }

    const char* lvl_name = ymo_yaml_node_as_str(
            ymo_yaml_object_get(root, "log_level"));
    if( lvl_name ) {
        ymo_log_set_level_by_name(lvl_name);
    }

    const ymo_yaml_node_t* wsgi_cfg = ymo_yaml_object_get(root, "wsgi");
    if( !wsgi_cfg ) {
        goto yaml_config_err;
    }

    /* TODO: tidy: */
    long y_long;
    const ymo_yaml_node_t* obj = NULL;
    if( (obj = ymo_yaml_object_get(wsgi_cfg, "port") ) ) {
        if( !ymo_yaml_node_as_long(obj, &y_long) ) {
            w_config->w_proc->port = y_long;
        } else {
            fprintf(stderr, "WSGI Config: malformed port: %s\n", ymo_yaml_node_as_str(obj));
            goto yaml_config_err;
        }
        obj = NULL;
    }

    if( (obj = ymo_yaml_object_get(wsgi_cfg, "no_proc") ) ) {
        if( !ymo_yaml_node_as_long(obj, &y_long) ) {
            w_config->w_proc->no_wsgi_proc = y_long;
        } else {
            fprintf(stderr, "WSGI Config: malformed no_proc: %s\n", ymo_yaml_node_as_str(obj));
            goto yaml_config_err;
        }
        obj = NULL;
    }

    if( (obj = ymo_yaml_object_get(wsgi_cfg, "no_threads") ) ) {
        if( !ymo_yaml_node_as_long(obj, &y_long) ) {
            w_config->w_proc->no_wsgi_threads = y_long;
        } else {
            fprintf(stderr, "WSGI Config: malformed no_threads: %s\n", ymo_yaml_node_as_str(obj));
            goto yaml_config_err;
        }
        obj = NULL;
    }

    /* This will be freed at teardown: */
    w_config->w_proc->cfg = doc;
    return;

yaml_config_err:
    w_config->w_proc->cfg = NULL;
    ymo_yaml_doc_free(doc);
    exit(-1);
    return;
}


static const char* config_from_optarg(ymo_wsgi_config_t* w_config, char yopt)
{
    optind = 0;

    char ch;
    const char* r_val = NULL;
    while((ch = getopt_long(w_config->argc, w_config->argv, "hl:c:P:p:t:", longopts, NULL)) != -1 )
    {
        switch( ch ) {
            case 'h':
                usage(stdout, w_config->argv[0]);
                exit(0);
                break;
            case 'l':
            case 'c':
            case 'P':
            case 'p':
            case 't':
                if( ch == yopt ) {
                    r_val = optarg;
                }
                break;
            default:
                usage(stderr, w_config->argv[0]);
                exit(-1);
                break;
        }
    }

    errno = 0;
    if( !w_config->w_proc->module || !w_config->w_proc->app ) {
        if( get_module_app(w_config->w_proc, w_config->argc-optind, w_config->argv+optind) ) {
            fprintf(stderr, "\nUnable to load module: %s", strerror(errno));
        }

        if( !w_config->w_proc->module || !w_config->w_proc->app ) {
            usage(stderr, w_config->argv[0]);
            exit(-1);
        }
    }
    return r_val;
}


int get_module_app(ymo_wsgi_proc_t* w_proc, int argc, char* const*argv)
{
    size_t buf_len = 0;

    for( int i = 0; i < argc; i++ )
    {
        buf_len += strlen(argv[i]);
    }

    char* module = malloc(buf_len+1);

    if( !module ) {
        errno = ENOMEM;
        return -1;
    }

    module[0] = '\0';
    char* dst = module;
    for( int i = 0; i < argc; i++ )
    {
        size_t len = strlen(argv[i]);
        strcat(dst, argv[i]);
        dst += len;
    }

    char* app = strchr(module, ':');
    if( app == NULL ) {
        free(module);
        errno = EINVAL;
        return -1;
    }

    *app++ = '\0';
    w_proc->module = module;
    w_proc->app = app;

    if( strlen(w_proc->module) == 0 || strlen(w_proc->app) == 0 ) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}


#endif /* YMO_WSGI_CONFIG */

