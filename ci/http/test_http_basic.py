#!/usr/bin/env python3
#===============================================================================
#
# NOTICE: THIS AUXILIARY FILE IS LICENSED USING THE MIT LICENSE.
#
# Copyright (c) 2021 Andrew T. Canaday
# This file is licensed under the MIT license.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
#-------------------------------------------------------------------------------

import os
import requests

YMO_HTTP_TEST_HOST = os.environ.get('YMO_HTTP_TEST_HOST', '127.0.0.1')
YMO_HTTP_TEST_PORT = os.environ.get('YMO_HTTP_TEST_HOST', '8081')
YMO_HTTP_TEST_URL_ROOT = os.environ.get(
        'YMO_HTTP_TEST_URL_ROOT',
        f'http://{YMO_HTTP_TEST_HOST}:{YMO_HTTP_TEST_PORT}')


def test_status():
    """Test basic status endpoint"""
    resp = requests.get(f'{YMO_HTTP_TEST_URL_ROOT}/status')
    assert resp.status_code == 200


def test_echo_headers():
    """
    Confirm that all the headers are parsed correctly,
    using the echo headers endpoint.
    """
    req_hdrs = {
            'custom-header': 'Custom-Value'
            }

    body = requests.get(
            f'{YMO_HTTP_TEST_URL_ROOT}/echo/headers', headers=req_hdrs).json()
    assert body['Custom-Header'] == 'Custom-Value'


def test_echo_body():
    """
    Confirm that body data is received and sent properly,
    using the echo body endpoint.
    """
    test_data = {
            'hello': 'world',
            'this': 'is some',
            'payload': 'data',
            }

    body = requests.post(
            f'{YMO_HTTP_TEST_URL_ROOT}/echo/body', json=test_data).json()

    for k,v in test_data.items():
        assert body[k] == v

