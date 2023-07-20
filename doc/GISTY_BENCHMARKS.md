# Gisty Benchmarks

> :construction: **These are _informal benchmarks and lack rigor_.** (They're
> mostly just for fun. :smile:).
>
> Notes:
>
> - Naturally, I posted the most flattering benchmarks. :stuck_out_tongue_winking_eye:
> - Results from wrk, seige, and go-wrk differ by ~ 5%.
> - I get better numbers with `gcc-10`, worse with `gcc-11` (difference < 1%)
> - RPS on my home Ubuntu Server, Core i7, 32GB RAM are ~ 60% of those below.
>
> Probably, I'll get some _real_ benchmarks put together when I stand up some
> CI. In the interim, here's the gist:


## Apache Bench, 2M clients, ..2 byte status payload.. (C Example Server)

- Compiler: gcc 11.2.0
- CFLAGS: `-Ofast`
- Hardware (compile and test):  Mac Mini (M1, 16GB RAM).
- Tested using: [Apache Bench](https://httpd.apache.org/docs/2.4/programs/ab.html).
-
(This one is a little silly. The only payload hooked up to the HTTP example
is a two-byte "OK" status endpoint... The RSS stays under `3,870` for the
duration of the test, though).

```
Document Path:          /status
Document Length:        0 bytes

Concurrency Level:      200
Time taken for tests:   8.027 seconds
Complete requests:      2000000
Failed requests:        1999999
   (Connect: 0, Receive: 36, Length: 1999963, Exceptions: 0)
Keep-Alive requests:    1999964
Total transferred:      179996760 bytes
HTML transferred:       3999928 bytes
Requests per second:    249151.64 [#/sec] (mean)
Time per request:       0.803 [ms] (mean)
Time per request:       0.004 [ms] (mean, across all concurrent requests)
Transfer rate:          21897.70 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.1      0       6
Processing:     0    1   0.2      1       7
Waiting:        0    1   0.2      1       7
Total:          0    1   0.2      1      12
```

## WSGI

### Apache Bench, 250k clients, 10k HTML Payload (WSGI)

> (:wave: **Reminder**: the overwhelming odds are that _your Python WSGI server
> is not the bottleneck in your production workloads!_)

- Compiler: Apple clang 12.0.5
- CFLAGS: `-O0`
- Hardware (compile and test):  Mac Mini (M1, 16GB RAM).
- Tested using: [Apache Bench](https://httpd.apache.org/docs/2.4/programs/ab.html).
- `YIMMO_WSGI_NO_PROC=4`
- `YIMMO_WSGI_NO_THREADS=1`

```
Document Path:          /payload/10k
Document Length:        10240 bytes

Concurrency Level:      64
Time taken for tests:   4.703 seconds
Complete requests:      250000
Failed requests:        0
Keep-Alive requests:    250000
Total transferred:      2583000000 bytes
HTML transferred:       2560000000 bytes
Requests per second:    53158.19 [#/sec] (mean)
Time per request:       1.204 [ms] (mean)
Time per request:       0.019 [ms] (mean, across all concurrent requests)
Transfer rate:          536357.85 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       2
Processing:     0    1   0.8      1      33
Waiting:        0    1   0.8      1      33
Total:          0    1   0.8      1      33
```

### K6 using ./ci/k6-ws-client-fast.js

#### `200` sessions, sending a random payload (128b to 4k) every 100ms

```
          /\      |‾‾| /‾‾/   /‾‾/
     /\  /  \     |  |/  /   /  /
    /  \/    \    |     (   /   ‾‾\
   /          \   |  |\  \ |  (‾)  |
  / __________ \  |__| \__\ \_____/ .io

  execution: local
     script: ./k6-ws-client-fast.js
     output: -

  scenarios: (100.00%) 1 scenario, 100 max VUs, 50s max duration (incl. graceful stop):
           * default: 100 looping VUs for 20s (gracefulStop: 30s)


     ✓ status is 101

     checks................: 100.00% ✓ 200         ✗ 0
     data_received.........: 435 MB  22 MB/s
     data_sent.............: 436 MB  22 MB/s
     iteration_duration....: avg=10s      min=10s     med=10s      max=10s    p(90)=10s    p(95)=10s
     iterations............: 200     9.992391/s
     vus...................: 100     min=100       max=100
     vus_max...............: 100     min=100       max=100
     ws_connecting.........: avg=5.97ms   min=3.43ms  med=4.6ms    max=8.67ms p(90)=8.43ms p(95)=8.52ms
     ws_msgs_received......: 200000  9992.391294/s
     ws_msgs_sent..........: 200032  9993.990076/s
     ws_ping...............: avg=695.93µs min=35.04µs med=432.06µs max=4.71ms p(90)=1.62ms p(95)=1.98ms
     ws_session_duration...: avg=10s      min=10s     med=10s      max=10s    p(90)=10s    p(95)=10s
     ws_sessions...........: 200     9.992391/s
```


#### 400 sessions, sending a random payload (128b to 4k) every 5ms

```
          /\      |‾‾| /‾‾/   /‾‾/
     /\  /  \     |  |/  /   /  /
    /  \/    \    |     (   /   ‾‾\
   /          \   |  |\  \ |  (‾)  |
  / __________ \  |__| \__\ \_____/ .io

  execution: local
     script: ./k6-ws-client-fast.js
     output: -

  scenarios: (100.00%) 1 scenario, 200 max VUs, 50s max duration (incl. graceful stop):
           * default: 200 looping VUs for 20s (gracefulStop: 30s)


     ✓ status is 101

     checks................: 100.00% ✓ 400          ✗ 0
     data_received.........: 1.1 GB  56 MB/s
     data_sent.............: 1.1 GB  56 MB/s
     iteration_duration....: avg=10.02s min=10s      med=10.01s max=10.07s  p(90)=10.04s  p(95)=10.05s
     iterations............: 400     19.906076/s
     vus...................: 200     min=200        max=200
     vus_max...............: 200     min=200        max=200
     ws_connecting.........: avg=6.96ms min=112.54µs med=6.08ms max=27.02ms p(90)=14.94ms p(95)=18.6ms
     ws_msgs_received......: 516184  25687.995049/s
     ws_msgs_sent..........: 516386  25698.047617/s
     ws_ping...............: avg=7.24ms min=1µs      med=3.37ms max=75.87ms p(90)=20.47ms p(95)=29.12ms
     ws_session_duration...: avg=10.02s min=10s      med=10.01s max=10.07s  p(90)=10.04s  p(95)=10.04s
     ws_sessions...........: 400     19.906076/s
```


#### 400 session, sending random payload (128b to 4k) every 1ms

```
          /\      |‾‾| /‾‾/   /‾‾/
     /\  /  \     |  |/  /   /  /
    /  \/    \    |     (   /   ‾‾\
   /          \   |  |\  \ |  (‾)  |
  / __________ \  |__| \__\ \_____/ .io

  execution: local
     script: ./k6-ws-client-fast.js
     output: -

  scenarios: (100.00%) 1 scenario, 200 max VUs, 50s max duration (incl. graceful stop):
           * default: 200 looping VUs for 20s (gracefulStop: 30s)


     ✓ status is 101

     checks................: 100.00% ✓ 400          ✗ 0
     data_received.........: 1.2 GB  59 MB/s
     data_sent.............: 1.2 GB  59 MB/s
     iteration_duration....: avg=10.06s  min=10s     med=10.04s  max=10.3s    p(90)=10.11s  p(95)=10.22s
     iterations............: 400     19.632413/s
     vus...................: 200     min=200        max=200
     vus_max...............: 200     min=200        max=200
     ws_connecting.........: avg=31.3ms  min=242.5µs med=13.1ms  max=221.75ms p(90)=70.31ms p(95)=100.43ms
     ws_msgs_received......: 552018  27093.614072/s
     ws_msgs_sent..........: 553464  27164.585246/s
     ws_ping...............: avg=37.16ms min=1.45µs  med=22.93ms max=373.33ms p(90)=86.66ms p(95)=124.16ms
     ws_session_duration...: avg=10.06s  min=10s     med=10.04s  max=10.29s   p(90)=10.11s  p(95)=10.22s
     ws_sessions...........: 400     19.632413/s
```
