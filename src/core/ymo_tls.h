/*=============================================================================
 * libyimmo: Lightweight socket server framework
 *
 * Copyright (c) 2014 Andrew Canaday
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




/** TLS Support
 * ============
 *
 *
 * .. note::
 *
 *    This header is a temporary measure to tidy up ymo_server/ymo_conn
 *    after some impromptu TLS hacking.
 *
 */

#ifndef YMO_TLS_H
#define YMO_TLS_H

#if YMO_ENABLE_TLS
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define CONN_SSL(conn) (conn->ssl)

/*---------------------------------------------------------------*
 *  Yimmo Server TLS Functions (HACK/POC):
 *---------------------------------------------------------------*/
static ymo_status_t ymo_init_ssl_ctx(ymo_server_t* server)
{
    ymo_log_notice("Configuring SSL context for port: %i", server->config.port);

    const SSL_METHOD* method;
    method = TLS_server_method();

    server->ssl_ctx = SSL_CTX_new(method);
    if( !server->ssl_ctx ) {
        ymo_log_warning("Unable to create SSL context: %s", strerror(errno));
        return errno;
    }

    int rc = SSL_CTX_use_certificate_file(
            server->ssl_ctx,
            server->config.cert_path,
            SSL_FILETYPE_PEM);
    ymo_log_notice("Cert config rc: %i", rc);
    if( rc <= 0 ) {
        ERR_print_errors_fp(stderr);
        return EINVAL;
    }

    rc = SSL_CTX_use_PrivateKey_file(
            server->ssl_ctx,
            server->config.key_path,
            SSL_FILETYPE_PEM);
    ymo_log_notice("Key config rc: %i", rc);
    if( rc <= 0 ) {
        ERR_print_errors_fp(stderr);
        return EINVAL;
    }

    return YMO_OKAY;
}


static inline ymo_status_t ymo_init_ssl(
        ymo_server_t* server, ymo_conn_t* conn, int client_fd)
{
    /* If running in SSL mode, init the SSL for the connection: */
    if( server->ssl_ctx ) {
        conn->ssl = SSL_new(server->ssl_ctx);

        if( !conn->ssl ) {
            ymo_log_warning(
                    "Failed to create SSL for connection on fd %i: %s",
                    client_fd, strerror(errno));
            return EPROTO;
        }

        if( !SSL_set_fd(conn->ssl, client_fd) ) {
            ymo_log_warning("Failed to set the SSL fd for %i", client_fd);
            return EPROTO;
        } else {
            conn->state = YMO_CONN_TLS_HANDSHAKE;
        }
    }

    return YMO_OKAY;
}


static inline ymo_status_t ymo_server_ssl_handshake(ymo_conn_t* conn)
{
    ERR_clear_error();

    /* If we haven't completed the SSL handshake, let's try now: */
    if( conn->state == YMO_CONN_TLS_HANDSHAKE ) {
        int accept_rc = 0;
        accept_rc = SSL_accept(conn->ssl);

        if( accept_rc == 1 ) {
            conn->state = YMO_CONN_TLS_ESTABLISHED;
            return YMO_OKAY;
        }

        int ssl_err = SSL_get_error(conn->ssl, accept_rc);
        if( YMO_SSL_WANT_RW(ssl_err) ) {
            ymo_log_debug("SSL handshake in progress on %i", conn->fd);
            return EAGAIN;
        } else {
            ymo_log_debug(
                    "Failed to initialize SSL for connection on fd %i with "
                    "return %i and error code %i: (%s)",
                    conn->fd, accept_rc, ssl_err,
                    ERR_error_string(ERR_get_error(), NULL));

            return ECONNABORTED;
        }
    }

    return YMO_OKAY;
}


static inline ssize_t ymo_server_ssl_read(
        ymo_server_t* server, ymo_conn_t* conn)
{
    ymo_status_t handshake_rc = ymo_server_ssl_handshake(conn);

    if( handshake_rc == YMO_OKAY ) {
        size_t bytes_read = 0;
        int read_rc = 0;

        read_rc = SSL_read_ex(conn->ssl, server->recv_buf,
                YMO_SERVER_RECV_BUF_SIZE, &bytes_read);

        if( read_rc == 1 ) {
            return bytes_read;
        }

        int ssl_err = SSL_get_error(conn->ssl, read_rc);
        if( YMO_SSL_WANT_RW(ssl_err) ) {
            ymo_log_debug("SSL buffering on read from %i", conn->fd);
            errno = EAGAIN;
        } else {
            ymo_log_debug(
                    "SSL read for connection on fd %i failed error code %i: (%s)",
                    conn->fd, ssl_err, ERR_reason_error_string(ssl_err));
            errno = EPROTO;
        }
    } else {
        errno = handshake_rc;
    }

    return -1;
}


#else /* !YMO_ENABLE_TLS */
#define CONN_SSL(conn) (NULL)

static inline ssize_t ymo_server_ssl_read(
        ymo_server_t* server, ymo_conn_t* conn)
{
    ymo_log_fatal(
            "SSL enabled on FD %i in non-TLS libyimmo build.",
            conn->fd);
    errno = EPROTONOSUPPORT;
    return -1;
}


#define ymo_init_ssl_ctx(s) (YMO_OKAY)
#define ymo_init_ssl(s, c, fd) (YMO_OKAY)

#endif /* YMO_ENABLE_TLS */
#endif /* YMO_TLS_H */

