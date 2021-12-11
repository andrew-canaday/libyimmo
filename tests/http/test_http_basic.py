#!/usr/bin/env python3
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

