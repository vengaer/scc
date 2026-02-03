.. _scc_rbmap:

``scc_rbmap``
=============

``scc_rbmap`` provides an associative red-black tree. The tree maintains each key-value pair
ordered by key and cannot at any point contain duplicates.

Like any red-black tree, the container guarantees insertion, removal and lookup at logarithmic complexity.

Type Declaration
----------------

.. doxygendefine:: scc_rbmap

.. doxygendefine:: scc_rbmap_iter

.. doxygentypedef:: scc_rmcompare

.. _rbmap_init:

Initialization
--------------

.. note::

    All ``rbmap`` instances, regardless of how they are created, should be passed to
    :ref:`scc_rbmap_free <scc_rbmap_free>` to ensure memory is reclaimed properly.

.. doxygendefine:: scc_rbmap_new

.. doxygendefine:: scc_rbmap_new_dyn

Destruction
-----------

.. doxygenfunction:: scc_rbmap_free

Modifiers
---------

.. doxygendefine:: scc_rbmap_insert

.. doxygendefine:: scc_rbmap_remove

.. doxygenfunction:: scc_rbmap_clear

Queries
-------

.. doxygendefine:: scc_rbmap_find

.. doxygenfunction:: scc_rbmap_size

.. doxygenfunction:: scc_rbmap_empty

Copying
-------

.. doxygendefine:: scc_rbmap_clone

Iteration
---------

.. doxygendefine:: scc_rbmap_foreach

.. doxygendefine:: scc_rbmap_foreach_reversed
