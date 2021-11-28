Multi-Processing
~~~~~~~~~~~~~~~~

If ``YIMMO_WSGO_NO_PROC==1``: you get the two threads described above.

If ``YIMMO_WSGI_NO_PROC > 1``, a **"manager process"** starts which spawns
``YIMMO_WSGI_NO_PROC`` worker processes, handles signals, and restarts failed
worker processes. This results in ``YIMMO_WSGI_NO_PROC + 1`` total processes,
but the main process spends most of its time sleeping.

Multi-processing reduces GIL contention. Generally, the GIL is really a
non-issue. On the other hand, it *does* take time to acquire and can be a real
bottleneck for CPU-bound tasks. In this case, *Python is the CPU-bound task* —
best bet is to set ``YIMMO_WSGI_NO_PROC`` to the number of cores you wish to
allow ``yimmo-wsgi`` to use at the same time.

.. figure:: /diagrams/yimmo-wsgi-spsw.svg
   :alt: Yimmo WSGI Single-Process, Single Thread Worker
   :align: center
   :target: /_images/yimmo-wsgi-spsw.svg

   Yimmo-WSGI configured with a single process worker and one thread worker.


.. figure:: /diagrams/yimmo-wsgi-mpsw.svg
   :alt: Yimmo WSGI Multiple Process Workers, Single Thread Worker
   :align: center
   :target: /_images/yimmo-wsgi-mpsw.svg

   Yimmo-WSGI configured with two process workers and one thread worker.



Multi-Threading
~~~~~~~~~~~~~~~

As stated earlier, every process has *at least* **two** threads. The
``YIMMO_WSGI_NO_THREADS`` environment variable is used to control the
number of **worker** threads *per process*.

Multi-threading *slightly* reduces the overall throughput (the threads can do
*a lot* in parallel, but they will still access the Python VM core serially, due
to the GIL). On the flip side, it distributes the HTTP request latency and
response times more equally — i.e. long-running requests are less likely to
block shorter requests, but some short requests will suffer here and there
from a context switch. With ``YIMMO_WSGI_NO_PROC < 12`` you won't notice too
much, though — if your load is consistent and requests are handled quickly —
you will definitely get your highest throughput by pinning threads to ``1``.

.. figure:: /diagrams/yimmo-wsgi-spmw.svg
   :alt: Yimmo WSGI Single-Process, Multiple Thread Workers
   :align: center
   :target: /_images/yimmo-wsgi-spmw.svg

   Yimmo-WSGI configured with a single process worker and multiple thread workers.

.. figure:: /diagrams/yimmo-wsgi-mpmw.svg
   :alt: Yimmo WSGI Multiple Process Workers, Multiple Thread Workers
   :align: center
   :target: /_images/yimmo-wsgi-mpmw.svg

   Yimmo-WSGI configured with two process workers and two thread workers per process.

