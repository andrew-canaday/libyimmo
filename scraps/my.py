#!/usr/bin/env python3
#==============================================================================
#
# A pretty bogus flask app that I use for loose testing of the WSGI server.
#
#------------------------------------------------------------------------------
import sys, pprint
import yimmo

def on_open(ws):
    print(f"{ws}: open!")

def on_message(ws, msg):
    print(f"{ws}: {msg}")

def on_close(ws):
    print(f"{ws}: closed!")

try:
    yimmo.init_websockets(on_open, on_message, on_close)
except:
    pass

USE_GEVENT=False

if USE_GEVENT:
    from gevent import monkey
    monkey.patch_all(thread=False)

import os
import sys
import time
import random
import json
import requests
import pprint

from flask import (
        Flask,
        request,
        Response,
        url_for,
        current_app,
        )

app = Flask(__name__)

PAYLOAD_1K = 'A' * 1024
PAYLOAD_10K = 'A' * (1024 * 10)
PAYLOAD_100K = 'A' * (1024 * 100)
PAYLOAD_1MB = 'A' * (1024 * 1024)

@app.route('/google')
def google():
    resp = requests.get("https://www.google.com")
    return (resp.text, resp.status_code, tuple())

@app.route('/1k')
def one_k():
    return PAYLOAD_1K

@app.route('/10k')
def ten_k():
    return PAYLOAD_10K

@app.route('/100k')
def hundred_k():
    return PAYLOAD_100K

@app.route('/1MB')
def one_mb():
    return PAYLOAD_1MB

@app.route('/self')
def get_self():
    resp = requests.get("http://127.0.0.1:8081/status")
    return (resp.text, resp.status_code, tuple())

@app.route('/sleep')
def sleepy_handler():
    # THIS CAUSES A SEGFAULT: time.sleep(0.25)
    time.sleep(0.01)
    return 'DID THIS WORK!?'

@app.route('/headers')
def headers_handler():
    s = ''
    for name,val in request.headers:
        s += f'{name}: {val}\n'
    return (s, 200, (('Fake-Header', 'Fake-Value'),('Other-Header', 'Other-Val')))
    #return json.dumps(dict(request.headers), indent=4)

@app.route('/chunked-sleep')
def chunked_sleep():
    def generate():
        for i in range(10):
            yield f'Time: {time.time()}\n'
            time.sleep(0.05)
    return Response(generate(), mimetype='text/plain')

@app.route('/dump-json')
def dump_json():
    my_data = {
            'text': "This is some text!",
            'int': 1234,
            'float': 3.14,
            'obj': {
                'This': 'is an object'
                },
            'list': ['this', 'is', 'a', 'list'],
            }
    return json.dumps(my_data)

@app.route('/status')
def self_status():
    return 'OK'

@app.route('/body', methods=['POST'])
def body_echo():
    body_data = request.get_data()
    return body_data

@app.route('/chunked')
def generate_chunked():
    def generate():
        for i in range(30):
            yield f'Hello, world ({i})\n'
    return Response(generate(), mimetype='text/plain')

@app.route('/stream-file/<fpath>')
def generate_file(fpath):
    def generate():
        with open(fpath, 'r') as f:
            for l in f.readlines():
                yield l
    return Response(generate(), mimetype='text/plain')

@app.route("/index.html")
def site_map():
    r = ''
    for url in app.url_map.iter_rules():
        r = r + f'- {url}\n'
    return r

@app.route('/py-info')
def py_info():
    return sys.version

@app.route('/r-info')
def r_info():
    return pprint.pformat(request.environ)

@app.route('/ws')
def ws():
    return WS_INDEX

if os.environ.get('YMO_WSGI_USE_WRITE', '') == 'true':
    flapp = app
    def app(environ, start_response):
        class _Wrap:
            def __init__(self):
                self.write = None

            def __call__(self, status, headers):
                self.write = start_response(status, headers)

        w = _Wrap()
        body = flapp(environ,w)
        for i in body:
            w.write(i)

WS_INDEX="""
<html>
    <head>
        <title>
            Libyimmo: WebSocket Test Page
        </title>
        <style>
            .container {
                border-radius: 5px;
                background-color: #F0F0F0;
                padding: 20px;
                width:600px;
                margin: 0 auto;
                margin-top: 5%;
                text-align: left;
                box-shadow: 20px 20px 15px #DDDDDD;
            }

            .status {
                border-radius: 5px;
                background-color: #AFA0AA;
                padding: 10px;
                width:300;
                margin: 0 auto;
                margin-top: 5%;
                text-align: left;
                box-shadow: 12px 12px 5px #777777;
            }

            .message {
                border-radius: 5px;
                background-color: #BBBBBB;
                padding: 8px;
                width:560;
                margin: 0 auto;
                margin-top: 5%;
                text-align: left;
                box-shadow: 3px 3px 5px #7A7A7A;
            }
        </style>
        <script>
            var no_lines = 0;
            var ws = new WebSocket("ws://127.0.0.1:8081/");
            ws.onopen = function() {
                var d = document.getElementById('msg_in');
                d.innerHTML += "<p class=\"status\">WebSocket: Open</p>";
                setInterval(function() {
                    if( ws.bufferedAmount == 0 ) {
                        ws.send(new Date().toUTCString());
                    }
                }, 500);
            }
            ws.onmessage = function(m) {
                var d = document.getElementById('msg_in');
                if( no_lines > 3 ) {
                    ws.close();
                    d.innerHTML += "<p class=\"status\">WebSocket: Closing</p>";
                }
                else {
                    no_lines++;
                    d.innerHTML += "<b><p class=\"message\">" + m.data + "</p></b>";
                };
            }
            ws.onclose = function() {
                var d = document.getElementById('msg_in');
                d.innerHTML += "<p class=\"status\">WebSocket: Closed</p>";
            }
        </script>
    </head>
    <body>
        <h1>Messages:</h1>
        <div class="container" id="msg_in">
        </div>
    </body>
</html>
"""

# EOF

