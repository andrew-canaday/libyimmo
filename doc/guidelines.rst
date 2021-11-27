Guide
=====

.. contents:: Table of Contents
   :local:
   :depth: 1


Source Layout
-------------------

The yimmo project source files are organized in the following manner.

.. container:: two-col

   .. list-table:: Overview
      :header-rows: 1
      :widths: auto

      * - Directory
        - Description
      * - ``include``
        - Contains header files for the :ref:`yimmo public API`
      * - ``lib``
        - Source and header files for libyimmo core
      * - ``mod``
        - Source and header files, by protocol.
      * - ``wsgi``
        - Source and header files for :ref:`yimmo-wsgi<maintainer_wsgi>`
      * - ``doc``
        - Source and makefiles for this documentation
      * - ``example``
        - Source and header files for example programs
      * - ``util``
        - Various utilities used during build, test, or for fun...
      * - ``m4``
        - Autoconf Macros (see `M4`_ and `Writing Autoconf Macros`_)
      * - ``benchmarks``
        - Source and header files for benchmarking (⚠ *all hacks!*)


   .. code-block::
      :caption: Layout

      libyimmo/
      │
      ├── include/     ───┐
      ├── lib/            ├─────  Core source
      │   ├── *.h/*.c     │       at top-level
      │   └── test/    ───┘
      │
      ├── mod/
      │   ├── <name>/       ───┐
      │   │   ├── include/     ├─────  Module source
      │   │   ├── *.h/*.c      │       under "mod"
      │   │   └── test/     ───┘
      │   │
      │   └── ...
      │
      ├── wsgi/      ───── Executables get their
      │                    own top-level directory
      └── ...


Modules
.......

At present, the modules are provided

.. list-table:: Shared libs
   :header-rows: 1
   :widths: auto

   * - Lib
     - Module
     - Description
   * - ``libyimmo``
     - :ref:`core<maintainer_core>`
     - Yimmo server, event core, and utilities.
   * - ``libyimmo_http``
     - :ref:`http<maintainer_http>`
     - HTTP 1.0/1.1
   * - ``libyimmo_ws``
     - :ref:`ws<maintainer_ws>`
     - `RFC6544`_ WebSockets
   * - ``libyimmo_mqtt``
     - :ref:`mqtt<maintainer_mqtt>`
     - *INCOMPLETE* `MQTT v3.1.1`_


WSGI
....

Additionally, this project provides an optional
`PEP 3333`_ compliant python WSGI
server (see `wsgi <../wsgi/README.md>`_ for more info).


Writing Docs
------------

- ``docs-local`` automake targets are provided by Makefiles in subdirs which have docs
- `c2sphinx`_ is used to generate RST/MD files from C sources
- `Sphinx`_ is used to generate documentation from RST/MD files
- `PlantUML`_ is used to generate diagrams


Documenting Source
..................

Source files may contain standalone doc commentss (interpretted as RST) or
doc comments immediately followed by code blocks (``.. code-block:: c``), e.g.:

.. code-block:: c

   /** My Heading
    * ------------
    *
    * This is a comment with no C declarations/definitions
    * immediately following it. It will be output almost
    * verbatim: only the C block header slashes and
    * asterisks will be removed.
    *
    * .. note::
    *    You can add the usual RST/Sphinx directives to
    *    comment blocks, and they'll work as expected!
    */

   /**
    * This struct is used to store whatever.
    * c2sphinx will figure out it's type and add the ``.. c:whatever:: stuff``.
    *
    * .. warning::
    *    Structs get special treatment! The whole
    *    definition is included in code fences.
    *    (This is just easier than a bunch of
    *    ``:var:`` declarations).
    */
   struct my_struct {
       void* whatever; /* We doc these anyway. */
       int   stuff;    /* May as well keep 'em. */
   };

   /** Functions
    *
    * :param s: a pointer to string
    * :param n: the number of characters
    * :returns: some size value 
    */
   size_t my_function(const char* s, size_t n);

   /** Typesdefs, etc, etc. */
   typedef int (*my_fn_type)(char c);

   /** Some macro. */
   #define SOME_MACRO 1

   /** Func-like macro. */
   #define SOME_FUNC(x,y) \
       (x+y)


Will produce the following RestructuredText output:

.. code-block:: rst

   My Heading
   ------------

   This is a comment with no C declarations/definitions
   immediately following it. It will be output almost
   verbatim: only the C block header slashes and
   asterisks will be removed.

   .. note::
      You can add the usual RST/Sphinx directives to
      comment blocks, and they'll work as expected!

   .. c:struct:: my_struct


      ```C
      struct my_struct {
          void* whatever; /* We doc these anyway. */
          int   stuff;    /* May as well keep 'em. */
      };
      ```

      This struct is used to store whatever.
      c2sphinx will figure out it's type and add the ``.. c:whatever:: stuff``.

      .. warning::
         Structs get special treatment! The whole
         definition is included in code fences.
         (This is just easier than a bunch of
         ``:var:`` declarations).

   .. c:function:: size_t my_function(const char* s, size_t n)

      Functions

      :param s: a pointer to string
      :param n: the number of characters
      :returns: some size value 

   .. c:type:: my_fn_type

      Typesdefs, etc, etc. 

   .. c:macro:: SOME_MACRO

      Some macro. 

   .. c:macro:: SOME_FUNC

      Func-like macro. 


Testing
-------

- |/| Unit tests (WIP)
- |x| Protocol tests (TODO)
- |_| Integration tests (TODO)

Unit Tests
..........

- Unit testing is done using the `Automake TAP Harness`_.
- The test "framework" is provided by `ymo_tap.h`_.
- Assertions are provided by `ymo_assert.h`_.


.. code-block:: c
   :caption: Example Unit Test

   /* Step 1: include ymo_tap.h */
   #include "ymo_tap.h"

   /* Step 2: (OPTIONAL!) Define a setup function: */
   static int my_setup(void)
   {
      fprintf(stderr, "%s\n(P.S. %s)\n",
          "This is an optional test initialization function.",
          "Diagnostic messages and logs should go to STDERR!");
      return 0;
   }

   /* Step 3: Define one or more tests: */
   static int test_demo_assertions(void)
   {
       /* Failed assertions will fail the test (these all pass): */
       ymo_assert(10 > 1);

       /* Indicate success with YMO_TAP_PASS: */
       YMO_TAP_PASS("Demo assertions worked out!");
   }

   /* Step 4: Run 'em!
    *
    * YMO_TAP_RUN will create a main function for us and invoke the tests:
    */
   YMO_TAP_RUN(NULL,
       YMO_TAP_TEST_FN(test_demo_assertions),
       YMO_TAP_TEST_END()
       )


The following is a set of guidelines aimed at creating and maintaining a
stable and predictable API.

Headers
-------

1. **License** information must be clearly displayed at the head of each
   file, according to the `GPL Guidelines`_.
2. `Include guards <http://en.wikipedia.org/wiki/Include_guard>`_ must be
   used for *all* header files.



Types
-----

Users should be provided access mediated by *nominally* opaque data types — i.e.
they should *appear* to be opaque, but:

1. We don't want to hide ``struct`` definitions from the compiler, if we can
   help it. LTO not withstanding, it's best to give the compiler as much info
   as possible, to maximize opportunity for optimization.
2. This is C. If the user is willing to get into the guts for good reason,
   let 'em (though, the consequences are theirs alone to bear).

"Nominally opaque" really means "through typedefs, documented in the public
API," e.g.:

.. code-block:: c

   /* The definition for this struct can
    * be in an internal API header OR
    * even defined in a public API header:
    */
    struct ymo_whatever {
       /* ...stuff... */
    };

   /* BUT — unless the user really needs
    * to access the fields (e.g. like with
    * ymo_http_request_t) this is the view
    * we present to the user:
    */

   /**
    * This is the type used to do whatever.
    */
   typedef struct ymo_whatever ymo_whatever_t;

   /**
    * This is how your perform some
    * operation on whatever.
    */
   ymo_whatever_some_operation(ymo_whatever_t* whatever);


Rules of thumb for exposing the user to data structure internals (for more info
see `Parameter Accessibility`_):

1. **Providing direct access to struct fields provides a more intuitive API**.
2. The structure contains relatively simple configuration data.
3. Direct field access from user code is absolutely necessary for performance.


Functions
---------

Parameter Order
...............

Functions which accept an *output parameter* should following the stdlib
convention - i.e, *destination on the left, source on the right* - ala the
the assignment operator, memcpy, et al.

Return Codes
............

Some predictability for return codes is required. For each general return type,
the error conditions should be indicated as follows:

.. list-table::
   :header-rows: 1
   :widths: auto

   * - Return Type
     - Error Indicator
   * - pointers
     - ``NULL`` on failure; setting ``errno`` appropriatel
   * - sizes
     - use `ssize_t < 0` on failure, setting ``errno`` appropriatel
   * - status codes
     - prefer ``ymo_status_t`` to ``int``, using ``errno.h`` status code

Parameter Accessibility
.......................

The API *should present opaque data structures* and access functions wherever
parameter access meets at least one of the following criteria:

1. Access is infrequent (e.g. server-initialization, session termination)
2. Function mediated access is necessary to osbscure implementation details
   which are dependent on configuration or in flux (e.g. maps, tries)
3. Direct access is likely to result in memory corruption or other
   catastrophic errors (e.g. struct representing server internals, parser
   state, etc).

Direct parameter access (through function parameters or publicly accessible
structure fields) should be provided when:

1. Fields are accessed frequently (e.g. request callbacks, encoding)
2. Implementation details are expected to remain relatively stable
   (e.g. request uri, content length)
3. Direct access is unlikely to have fatal outcomes (e.g. HTTP status)


Optimization
------------

1. **Document it!!** Documentation *cannot be too explicit*.
   Include assumptions and domain limitations.
2. If it is easily contained in a single function *and* clearly provides
   a reduction in memory/cpu overhead: *go ahead*. (See Rule #1!)
3. *Back it up with profiling/benchmarking!* (See Rule #1!)
4. If the optimization relies on any *compiler-specific* behavior or behavior
   which the standard indicates has an *undefined result*, it **must** meet
   the criteria for #3 *and* be protected by preproc which constrains its
   compilation to *only those platform/compiler combinations for which it
   is supported!*. (See Rule #1!)


.. note::
   Obviously, optimization can be really fun. Fun is worthwhile. Please, feel
   free to experiment.

Misc
----


Notes
.....

.. toctree::
   :maxdepth: 1

   notes/TODO
   notes/IDEAS


.. _Automake TAP Harness: https://www.gnu.org/software/automake/manual/html_node/Use-TAP-with-the-Automake-test-harness.html
.. _ymo_tap.h: core/ymo_tap_h.html
.. _ymo_assert.h: core/ymo_assert_h.html
