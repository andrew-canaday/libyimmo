#!/usr/bin/env python3

import os
from flask import (
    Flask,
    Response,
    request,
    send_file,
)
import time
import datetime

# ------------------------
# NOTE:
#
# - It's not necessary to wrap yimmo in a try/except. This is just a hack
#   to allow the documentation to be built whether the yimmo module is or
#   isn't.
# - You don't need to import yimmo to use the WSGI server. This is here
#   pretty much exclusively for the WebSockets demo.
#
try:
    import yimmo
except:
    yimmo = None

# Create a simple flask app.
# (See https://flask.palletsprojects.com/en/2.0.x/quickstart/)
app = Flask(__name__)


@app.route('/status')
def status_ok():
    """
    Status endpoint. We'll just use this to serve "OK" and as a check that
    the service is up and running.
    """
    return 'OK'


# ---------------------------------------------------
# NOTE: The following endpoints are mostly used for
#       testing purposes!
# ---------------------------------------------------
@app.route('/echo/headers')
def headers_handler():
    """
    Send the received HTTP request headers back as a JSON dictionary.
    """
    return dict(request.headers)


@app.route('/echo/body', methods=['POST'])
def body_echo():
    """
    Echo back the HTTP POST body, verbatim.
    """
    return Response(request.get_data(), headers={
        'Content-Type': request.headers['Content-Type']
    })


@app.route('/source')
def get_source():
    """
    Serve the source code for this app from disk.
    """
    file_path = os.path.realpath(__file__)
    return send_file(file_path)


@app.route('/chunked')
def get_chunks():
    """
    Return a series of timestamps with some delay_ms, as HTTP chunks.
    """

    # Number of chunks:
    no_chunks_default = 10
    no_chunks = request.args.get('no_chunks', no_chunks_default)
    try:
        no_chunks = int(no_chunks)
    except:
        no_chunks = no_chunks_default

    # Delay between chunks:
    delay_ms_default = 0.0
    delay_ms = request.args.get('delay_ms', delay_ms_default)
    try:
        delay_ms = (float(delay_ms) / 1000.0)
    except:
        delay_ms = delay_ms_default

    # --------------
    # app-chunked:
    #
    # - If the 'app-chunked' flag is unset or false, yimmo will chunk the
    #   response automatically, if Content-Length is set or — in the case that
    #   it is not — if flask returns a single-item iterable.
    #
    # - If the 'app-chunked' flag is set, we generate the payload ahead of
    #   time, and set the 'Transfer-Encoding' header manually. Yimmo should
    #   handle this by just passing the data straight through.
    use_generator = True
    app_chunked = request.args.get('app-chunked', 'false')
    if(app_chunked.lower() == 'true'):
        use_generator = False

    # ----------------------
    # Create the response:
    def create_chunked_response():
        for i in range(1,no_chunks+1):
            y_str = f'Chunk#: {i: 4d}\n' + ('#' * (i%80)) + '\n'
            # print(f'\033[00;32;mNext chunk: {y_str}\033[00;m', end='')
            if delay_ms:
                time.sleep(delay_ms)
            yield y_str
        # TODO: chunk terminal not sent without this None return.
        #       (Probably, that means we're not catching StopIteration).
        return None

    if use_generator:
        return app.response_class(
            create_chunked_response(),
            mimetype='text/plain',
        )
    else:
        s = ''.join(f'{len(c):x}\r\n{c}' for c in create_chunked_response()) \
            + '0\r\n\r\n'
        return app.response_class(s, mimetype='text/plain')


# ---------------------------------------------------
# HACK HACK: toss together some endpoints with
#            payload data.
#
# NOTE: This is no-good for benchmarking!
#       (All in mem / no disk IO, etc).
# ---------------------------------------------------
PAYLOAD_10K = 'A' * 1024 * 10
@app.route('/payload/10k')
def payload_10k():
    return Response(PAYLOAD_10K, headers={
        'Content-Type': 'text/plain'
    })


# ---------------------------------------------------
# WebSockets endpoints/code:
# ---------------------------------------------------
@app.route('/websocket')
def get_ws_index():
    """
    Serve back a very basic HTML page with some embedded
    JavaScript that uses the WebSockets API.
    """
    file_path = os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        "index.html")
    return send_file(file_path)


# -----------------------
# WebSockets Callbacks:
# -----------------------

def on_open(ws):
    """
    Invoked when a new websocket connection is established.

    :param ws: the yimmo.WebSocket connection
    :return: on success, :py:obj:`None` or ``0``
    :return: on failure, a non-zero integer to set ``errno``
    :return: any other return value is interpretted as "failure"

    If an exception is raised or anything other than :py:obj:`None` or
    ``0`` is returned, the connection is closed.
    """
    return


def on_message(ws, msg, flags):
    """
    Invoked when a message is received.

    :param ws: the yimmo.WebSocket connection
    :param flags: WebSocket message frame flags
    :return: on success, :py:obj:`None` or ``0``
    :return: on failure, a non-zero integer to set ``errno``
    :return: any other return value is interpretted as "failure"

    If an exception is raised or anything other than :py:obj:`None` or
    ``0`` is returned, the connection is closed.
    """
    ws.send(msg, flags)
    return


def on_close(ws):
    """
    Invoked after a websocket disconnect.

    :param ws: the yimmo.WebSocket connection

    Return values are ignored. Any exceptions raised are swallowed and
    ignored.
    """
    return


# If WebSocket support is enabled, let's go ahead and
# set up some WebSocket handlers:
if hasattr(yimmo, 'WebSocket'):
    yimmo.init_websockets(on_open, on_message, on_close)
