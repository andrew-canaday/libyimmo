#!/usr/bin/env python3

import os
from flask import (
        Flask,
        Response,
        request,
        send_file,
        )

#------------------------
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


#---------------------------------------------------
# NOTE: The following endpoints are mostly used for
#       testing purposes!
#---------------------------------------------------
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


#---------------------------------------------------
# WebSockets endpoints/code:
#---------------------------------------------------
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


#-----------------------
# WebSockets Callbacks:
#-----------------------

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
