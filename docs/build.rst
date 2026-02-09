Building
========

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
-----------

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
--------------

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
