.. _gdeque:

``scc_deque``
=============

``scc_deque`` implements a consecutive, double-ended queue. Values may be pushed
to either the :ref:`front <scc_deque_push_front>` or the
:ref:`back <scc_deque_push_back>`.

Type Declaration
----------------

.. doxygendefine:: scc_deque

Configuration
-------------

.. doxygendefine:: SCC_DEQUE_STATIC_CAPACITY

Initialization
--------------

.. note::

    All ``deque`` instances, regardless of how they are created, should be passed to
    :ref:`scc_deque_free <scc_deque_free>` to ensure memory is reclaimed properly.

.. doxygendefine:: scc_deque_new

.. doxygendefine:: scc_deque_new_dyn

Destruction
-----------

.. doxygenfunction:: scc_deque_free

Modifiers
---------

.. doxygendefine:: scc_deque_push_front

.. doxygendefine:: scc_deque_push_back

.. doxygendefine:: scc_deque_pop_front

.. doxygendefine:: scc_deque_pop_back

.. doxygenfunction:: scc_deque_clear

.. doxygendefine:: scc_deque_reserve

Queries
-------

.. doxygenfunction:: scc_deque_capacity

.. doxygenfunction:: scc_deque_size

.. doxygenfunction:: scc_deque_empty

.. doxygendefine:: scc_deque_front

.. doxygendefine:: scc_deque_back

Copying
-------

.. doxygendefine:: scc_deque_clone
