.. _ringdeque:

``scc_ringdeque``
=================

``scc_ringdeque`` implements a consecutive, double-ended queue. Values may be pushed
to either the :ref:`front <scc_ringdeque_push_front>` or the
:ref:`back <scc_ringdeque_push_back>`.

Type Declaration
----------------

.. doxygendefine:: scc_ringdeque

Configuration
-------------

.. doxygendefine:: SCC_RINGDEQUE_STATIC_CAPACITY

Initialization
--------------

.. note::

    All ``deque`` instances, regardless of how they are created, should be passed to
    :ref:`scc_ringdeque_free <scc_ringdeque_free>` to ensure memory is reclaimed properly.

.. doxygendefine:: scc_ringdeque_new

.. doxygendefine:: scc_ringdeque_new_dyn

Destruction
-----------

.. doxygenfunction:: scc_ringdeque_free

Modifiers
---------

.. doxygendefine:: scc_ringdeque_push_front

.. doxygendefine:: scc_ringdeque_push_back

.. doxygendefine:: scc_ringdeque_pop_front

.. doxygendefine:: scc_ringdeque_pop_back

.. doxygenfunction:: scc_ringdeque_clear

.. doxygendefine:: scc_ringdeque_reserve

Queries
-------

.. doxygenfunction:: scc_ringdeque_capacity

.. doxygenfunction:: scc_ringdeque_size

.. doxygenfunction:: scc_ringdeque_empty

.. doxygendefine:: scc_ringdeque_front

.. doxygendefine:: scc_ringdeque_back

Copying
-------

.. doxygendefine:: scc_ringdeque_clone
