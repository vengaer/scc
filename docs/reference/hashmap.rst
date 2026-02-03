.. _hashmap:

``scc_hashmap``
===============

``scc_hashmap`` is a `SIMD`_-accelerated, associative, hash-based container resolving collisions
using open addressing. The default implementation uses an `FNV-1a`_ hash function.

.. _SIMD: https://en.wikipedia.org/wiki/Single_instruction,_multiple_data

.. _FNV-1a: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

Type Declaration
----------------

.. doxygendefine:: scc_hashmap

.. doxygentypedef:: scc_hashmap_eq

.. doxygentypedef:: scc_hashmap_hash

Configuration
-------------

.. doxygendefine:: SCC_HASHMAP_STACKCAP

.. _hashmap_init:

Initialization
--------------

.. note::

    All ``hashmap`` instances, regardless of how they are created, should be passed to
    :ref:`scc_hashmap_free <scc_hashmap_free>` to ensure memory is reclaimed properly.

.. doxygendefine:: scc_hashmap_new

.. doxygendefine:: scc_hashmap_new_dyn

.. doxygendefine:: scc_hashmap_with_hash

.. doxygendefine:: scc_hashmap_with_hash_dyn

Destruction
-----------

.. doxygenfunction:: scc_hashmap_free

Modifiers
---------

.. doxygendefine:: scc_hashmap_insert

.. doxygendefine:: scc_hashmap_remove

.. doxygenfunction:: scc_hashmap_clear

..
    _TODO: implement reserve

Queries
-------

.. doxygendefine:: scc_hashmap_find

.. doxygenfunction:: scc_hashmap_capacity

.. doxygenfunction:: scc_hashmap_size

Copying
-------

.. doxygenfunction:: scc_hashmap_clone

Iteration
---------

..
    _TODO: implement
