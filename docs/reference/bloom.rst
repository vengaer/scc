.. _bloom:

``scc_bloom``
=============


``scc_bloom`` implements a `bloom filter`_ using 5 `MurmurHash`_-based hash functions.

.. _MurmurHash: https://en.wikipedia.org/wiki/MurmurHash

.. _bloom filter: https://en.wikipedia.org/wiki/Bloom_filter

Type Declarations
-----------------

.. doxygendefine:: scc_bloom

.. doxygentypedef:: scc_bloom_hash

Initialization
--------------

.. doxygendefine:: scc_bloom_new

.. doxygendefine:: scc_bloom_new_dyn

Destruction
-----------

.. doxygenfunction:: scc_bloom_free

Modifiers
---------

.. doxygendefine:: scc_bloom_insert

Queries
-------

.. doxygendefine:: scc_bloom_test

.. doxygenfunction:: scc_bloom_capacity

.. doxygenfunction:: scc_bloom_nhashes
