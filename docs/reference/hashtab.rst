.. _hashtab:

``scc_hashtab``
===============

``scc_hashtab`` is a `SIMD`_-accelerated, hash-based container resolving collisions
using open addressing. The default implementation uses an `FNV-1a`_ hash function.

.. _SIMD: https://en.wikipedia.org/wiki/Single_instruction,_multiple_data

.. _FNV-1a: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

Type Declaration
----------------

.. doxygendefine:: scc_hashtab

.. doxygendefine:: scc_hashtab_iter

.. doxygentypedef:: scc_hashtab_eq

.. doxygentypedef:: scc_hashtab_hash

Configuration
-------------

.. doxygendefine:: SCC_HASHTAB_STACKCAP

.. _hashtab_init:

Initialization
--------------

.. note::

    All ``hashtab`` instances, regardless of how they are created, should be passed to
    :ref:`scc_hashtab_free <scc_hashtab_free>` to ensure memory is reclaimed properly.

.. doxygendefine:: scc_hashtab_new

.. doxygendefine:: scc_hashtab_new_dyn

.. doxygendefine:: scc_hashtab_with_hash

.. doxygendefine:: scc_hashtab_with_hash_dyn

Destruction
-----------

.. doxygenfunction:: scc_hashtab_free

Modifiers
---------

.. doxygendefine:: scc_hashtab_insert

.. doxygendefine:: scc_hashtab_remove

.. doxygendefine:: scc_hashtab_reserve

.. doxygenfunction:: scc_hashtab_clear

Queries
-------

.. doxygendefine:: scc_hashtab_find

.. doxygenfunction:: scc_hashtab_capacity

.. doxygenfunction:: scc_hashtab_size

Copying
-------

.. doxygenfunction:: scc_hashtab_clone

Iteration
---------

.. doxygendefine:: scc_hashtab_foreach
