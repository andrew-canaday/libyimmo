import ws from "k6/ws";
import { check } from "k6";

export const options = {
  duration: '20s',
  vus: 200,
};

export default function () {
    var url = "ws://127.0.0.1:8081/status";
    var params = { "tags": { "my_tag": "hello" } };

    var response = ws.connect(url, params, function (socket) {
        socket.on('open', function open() {
            socket.send(Date.now());

            socket.setInterval(function timeout() {
                var no_bytes = Math.floor(Math.random()*4096)+128;
                var payload = new Array(no_bytes).join( "#" );
                socket.send(payload);
            }, 1);

            socket.setInterval(function timeout() {
                socket.ping();
            }, 1000);
        });

        socket.on('ping', function () {
        });

        socket.on('pong', function () {
        });

        socket.on('message', function incoming(data) {
            /*
            socket.setTimeout(function timeout() {
                socket.send(Date.now());
            }, 500);
            */
        });

        socket.on('close', function close() {
        });

        socket.on('error', function (e) {
            if (e.error() != "websocket: close sent") {
                console.log('An unexpected error occurred: ', e.error());
            }
        });

        socket.setTimeout(function () {
            socket.close();
        }, 10000);
    });

    check(response, { "status is 101": (r) => r && r.status === 101 });
};
