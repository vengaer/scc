.. _scc_stack:

``scc_stack``
=============

``scc_stack`` provides the standard LIFO semantics expected by a stack. The implementation is built on
top of :ref:`scc_vec <scc_vec>`.

Type Declaration
----------------

.. doxygendefine:: scc_stack

.. _stack_init:

Initialization
--------------

.. note::

    All ``stack`` instances, regardless of how they are created, should be passed to
    :ref:`scc_stack_free <scc_stack_free>` to ensure memory is reclaimed properly.

.. doxygendefine:: scc_stack_new

.. doxygendefine:: scc_stack_new_dyn

Destruction
-----------

.. doxygendefine:: scc_stack_free

Modifiers
---------

.. doxygendefine:: scc_stack_push

.. doxygendefine:: scc_stack_pop

.. doxygendefine:: scc_stack_clear

.. doxygendefine:: scc_stack_reserve

Queries
-------

.. doxygendefine:: scc_stack_top

.. doxygendefine:: scc_stack_size

.. doxygendefine:: scc_stack_empty

.. doxygendefine:: scc_stack_capacity

Copying
-------

..
    _TODO: implement

Iteration
---------

..
    _TODO: implement
