.. _scc_rbtree:

``scc_rbtree``
==============

``scc_rbtree`` provides an straightforward red-black tree. The elements are ordered in accordance with the
supplied :ref:`comparator <scc_rbcompare>`.

Like any red-black tree, the container guarantees insertion, removal and lookup at logarithmic complexity.

Type Declaration
----------------

.. doxygendefine:: scc_rbtree

.. doxygendefine:: scc_rbtree_iter

.. doxygentypedef:: scc_rbcompare

.. _rbtree_init:

Initialization
--------------

.. note::

    All ``rbtree`` instances, regardless of how they are created, should be passed to
    :ref:`scc_rbtree_free <scc_rbtree_free>` to ensure memory is reclaimed properly.

.. doxygendefine:: scc_rbtree_new

.. doxygendefine:: scc_rbtree_new_dyn

Destruction
-----------

.. doxygenfunction:: scc_rbtree_free

Modifiers
---------

.. doxygendefine:: scc_rbtree_insert

.. doxygendefine:: scc_rbtree_remove

.. doxygenfunction:: scc_rbtree_clear

Queries
-------

.. doxygendefine:: scc_rbtree_find

.. doxygenfunction:: scc_rbtree_size

.. doxygenfunction:: scc_rbtree_empty

Copying
-------

.. doxygendefine:: scc_rbtree_clone

Iteration
---------

.. doxygendefine:: scc_rbtree_foreach

.. doxygendefine:: scc_rbtree_foreach_reversed
