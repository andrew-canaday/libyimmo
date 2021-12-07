Musings /Scratch Pad
====================

Scratch pad of design ideas and considerations (**needs pruning!**)

.. contents:: Contents
   :local:
   :depth: 2

Misc
----

- |_| Use `ecb.h <http://cvs.schmorp.de/libecb/ecb.h>`_?
- |_| Timed-idle watchers — do stuff at least-so often, if not sooner.
- |_| Dynamic loader for pluggable protocols
- |_| add support for edge-triggered IO (i.e. keep **recv'ing** until block)?
- |_| Move "server" code - socket(), bind(), etc - into *optional* server
       functionality. i.e. The library should just provide the IO, for the
       most part.
- |_| where appropriate utilize `feature test macros <https://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html>`_

Network IO
----------

Polling
.......
- [x] libev (current)
- [ ] libapr_poll_t (why?)
- [ ] nanomsg (why?)

SIGPIPE
.......
- Globally (signal/sigaction):
    - use signal/sigaction
    - most ubiquitious/easiest
    - least friendly to user code
    - possibly slowest?
- Thread-wide (pthread_sigmask)
- At the socket level, ``POSIX``-style (fcntl w/ ``F_GETNOSIGPIPE``/``F_SETNOSIGPIPE``)
- At the socket level, BSD-style (setsockopt w/ ``SO_NOSIGPIPE``)
- At the socket level, SysV-style (no possible via ioctl???)
- At read/write time (``MSG_NOSIGNAL``)
    - requires send/recv/sendmsg/recvmsg over write/read/writev/readv
    - not portable?

Notes
^^^^^

- on Darwin ``F_SETNOSIGPIPE`` invokes setsockopt w/ ``SO_NOSIGPIPE`` under the covers
- the file-level ``O_NONBLOCK`` flag is used by the kernel to set ``MSG_DONTWAIT`` at
   send/recv time!! (see net/socket.c)
- the ``FIONBIO`` ioctl call results in ``O_NONBLOCK`` fd-level flag being set
   (see fs/ioctl.c) 
- Mac OS X *does* have ``MSG_DONTWAIT``, etc. It's only for ``DARWIN`` or Non-``POSIX``
   sources.
- Also, look into xnu and linux sources for other goodies such as:
    - ``MSG_NBIO``
    - ``FNONBLOCK``
    - ``O_NDELAY``
    - etc
- All relevant non-blocking flags, to date:
    - ``FIONBIO``
    - ``FNONBLOCK``
    - ``MSG_NBIO``
    - ``MSG_DONTWAIT``
    - ``O_NDELAY``
    - ``O_NONBLOCK``
- Sending files:
    - Use **sendfile**, where available
    - Use mmqp + write, otherwise

Flow control
............
- ``TCP_NODELAY`` for urgent packets (when/why?)
- ``TCP_CORK`` for manual packet coalescing (when/why?)

Performance
-----------

- Use computed goto's instead of switch statements?
- NO: Consider page locking to prevent the most common page faults (that's super
      rude, though...)
- NO: Consider interacting with the scheduler via `sched.h <http://www.gnu.org/software/corec/manual/html_mono/corec.html#Priority>`_

Protocols
---------

- Which pre-standard versions of websockets are supported?
    - |x| hybi-10
    - |x| hybi-7
    - |x| hybi-00
    - |x| hixie-76
    - |x| hixie-75
- |_| Support HTTP/2.0
    - `HTTP/2.0 <https://datatracker.ietf.org/doc/html/draft-ietf-httpbis-http2-17>`_
    - `HPACK <https://datatracker.ietf.org/doc/html/draft-ietf-httpbis-header-compression-12>`_
- |_| (?) Support `QUIC <https://github.com/devsisters/corequic>`_

