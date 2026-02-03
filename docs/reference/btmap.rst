.. _btmap:

``scc_btmap``
=============

``scc_btmap`` implements an associative `B-tree`_ --- effectively a self-balancing N-ary search
tree --- of arbitrary order.

.. _B-tree: https://en.wikipedia.org/wiki/B-tree

Type Declarations
-----------------

.. doxygendefine:: scc_btmap

.. doxygentypedef:: scc_btmcompare

Configuration
-------------

.. doxygendefine:: SCC_BTMAP_DEFAULT_ORDER

Initialization
--------------

.. doxygendefine:: scc_btmap_new

.. doxygendefine:: scc_btmap_new_dyn

.. doxygendefine:: scc_btmap_with_order

.. doxygendefine:: scc_btmap_with_order_dyn

Destruction
-----------

.. doxygenfunction:: scc_btmap_free

Modifiers
---------

.. note::

    Several modifiers reallocate the ``btmap`` instance and thus require that the address of
    the ``btmap`` rather than the ``btmap`` itself is passed.

    This section makes a distrinction between parameters named ``btmap`` and ``btmapaddr``. The
    former refers to the handle stored in the :ref:`scc_btmap <scc_btmap>` instance, the latter
    to the address of said instance.

.. doxygendefine:: scc_btmap_insert

.. doxygendefine:: scc_btmap_remove

Queries
-------

.. doxygendefine:: scc_btmap_find

.. doxygenfunction:: scc_btmap_order

.. doxygenfunction:: scc_btmap_size

Copying
-------

.. doxygenfunction:: scc_btmap_clone

Iteration
---------

..
    _TODO: implement
