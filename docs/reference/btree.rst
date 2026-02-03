``scc_btree``
=============

``scc_btree`` is a `B-tree`_ --- effectively a self-balancing N-ary search
tree --- implementation with support for arbitrary order. As opposed to
:ref:`scc_btmap <btmap>`, ``scc_btree`` supports duplicate entries.

.. _B-tree: https://en.wikipedia.org/wiki/B-tree

Type Declarations
-----------------

.. doxygendefine:: scc_btree

.. doxygentypedef:: scc_btcompare

Configuration
-------------

.. doxygendefine:: SCC_BTREE_DEFAULT_ORDER

.. _btree_init:

Initialization
--------------

.. doxygendefine:: scc_btree_new

.. doxygendefine:: scc_btree_new_dyn

.. doxygendefine:: scc_btree_with_order

.. doxygendefine:: scc_btree_with_order_dyn

Destruction
-----------

.. doxygenfunction:: scc_btree_free

Modifiers
---------

.. doxygendefine:: scc_btree_insert

.. doxygendefine:: scc_btree_remove

Queries
-------

.. doxygendefine:: scc_btree_find

.. doxygenfunction:: scc_btree_order

.. doxygenfunction:: scc_btree_size

Copying
-------

.. doxygendefine:: scc_btree_clone

Iteration
---------

..
    _TODO: implement
