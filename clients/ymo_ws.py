#!/usr/bin/env python
#==============================================================================
# Example WebSocket (RFC6455) client for libyimmo
#==============================================================================

import os
import json
import pprint

import asyncio
import websockets

# input params:
YMO_DEFAULT_ENDPOINT='ws://127.0.0.1:8081'
ymo_ws_url=os.environ.get('YMO_DEFAULT_ENDPOINT', YMO_DEFAULT_ENDPOINT)


async def hello():
    async with websockets.connect(ymo_ws_url) as websocket:
        for i in range(int(os.environ.get('YMO_WS_NO_CLIENTS', 10))):
            await websocket.send("Client msg {}".format(i+1))
            msg_resp = await websocket.recv()
            print(f"{msg_resp}")
        await websocket.close()

if __name__ == '__main__':
    try:
        asyncio.get_event_loop().run_until_complete(hello())
    except KeyboardInterrupt:
        client.close()

# EOF

