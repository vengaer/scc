Building
========

scc sports two build systems --- a GNU make-based one used during development and a
cmake-based counterpart recommended for end users. This page is limited to describe
only the latter of the two.

Recommended Approach
--------------------

The recommended approach for building scc is using cmake. You may, of course, use
either GNU Make or Ninja.

Headers are installed in a separate subdirectory called ``scc`` whereas the libs
are installed directly under ``lib``. Provided that ``CMAKE_INSTALL_PREFIX`` is set
to /usr/local, the installed files are ordered as follows

::

    /usr/local
    ├── include
    │   └── scc
    │       ├── algorithm.h
    │       ├── arch.h
    │       ├── arena.h
    │       ├── bits.h
    │       ├── bloom.h
    │       ├── btmap.h
    │       ├── btree.h
    │       ├── bug.h
    │       ├── canary.h
    │       ├── config.h
    │       ├── deque.h
    │       ├── hash.h
    │       ├── hashmap.h
    │       ├── hashtab.h
    │       ├── mem.h
    │       ├── perf.h
    │       ├── pp_token.h
    │       ├── rbmap.h
    │       ├── rbtree.h
    │       ├── stack.h
    │       ├── swar.h
    │       └── vec.h
    └── lib
       ├── libscc.a
       └── libscc.so

Using Ninja
***********

.. code-block:: bash
    :caption: Building and installing scc under /usr/local, using Ninja

    mkdir build
    cd build
    cmake -GNinja \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          -DCMAKE_BUILD_TYPE=Release \
          ..
    ninja -j$(nproc)
    sudo ninja install

Using GNU Make
**************

.. code-block:: bash
    :caption: Building and installing scc under /usr/local, using GNU Make

    mkdir build
    cd build
    cmake -G'Unix Makefiles' \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          -DCMAKE_BUILD_TYPE=Release \
          ..
    make -j$(nproc)
    sudo make install

Configuration Options
---------------------

The library may be configured to not only reduce external dependencies but also
to prevent use of certain hardware features. This section endeavors to cover the
various configuration options exposed to the build system.

Manage libm Dependency
**********************

The bloom filter implementation provides a :ref:`scc_bloom_size <scc_bloom_size>` that
approximates the number of values stored in the filter. This approximation requires
functions provided by libm which may not be available on e.g. smaller embedded systems.

The libm dependency may be disabled by passing ``-DSCC_NO_LIBM=ON`` to cmake.

Disable SIMD
************

When compiled for x86_64 systems using the System V ABI, scc will by default use
AVX2-accelerated lookup for its hash tables. Support for features such as OSXSAVE ,
AVX and YMM state is determined lazily at runtime.

While the library handles all of the above logic, users may nevertheless wish to
disable SIMD support. This is achieved by passing ``-DSCC_SIMD=OFF`` to cmake.
