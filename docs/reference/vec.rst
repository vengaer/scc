.. _scc_vec:

``scc_vec``
===========

``scc_vec`` provides implements a generic, dynamic array in the vein of C++'s
``std::vector`` and Rust's ``std::vec``. ``vec`` handles are implemented in a
way that allows the underlying array to be indexed directly using ``[]``.

.. literalinclude:: /../examples/vec/rs_vec_ex.c
    :caption: Example use of ``scc_vec``
    :start-after: int main
    :end-at: assert(!memcmp
    :language: c

Type Declarations
-----------------

.. doxygendefine:: scc_vec

.. doxygendefine:: scc_vec_iter

Configuration
-------------

.. doxygendefine:: SCC_VEC_STATIC_CAPACITY

Initialization
--------------

.. doxygendefine:: scc_vec_new

.. doxygendefine:: scc_vec_new_dyn

.. doxygendefine:: scc_vec_from

.. doxygendefine:: scc_vec_from_dyn

Destruction
-----------

.. doxygenfunction:: scc_vec_free

Modifiers
---------

.. note::

    Several modifiers may need to reallocate the ``vec`` instance and do thus require that
    the address of the ``vec`` rather than the ``vec`` itself is passed.

    This section makes a distrinction between parameters named ``vec`` and ``vecaddr``. The
    former refers to the handle stored in the :ref:`scc_vec <scc_vec>` instance, the latter
    to the address of said instance.

.. doxygendefine:: scc_vec_push

.. doxygendefine:: scc_vec_pop

.. doxygendefine:: scc_vec_resize

.. doxygendefine:: scc_vec_reserve

.. doxygendefine:: scc_vec_erase

.. doxygendefine:: scc_vec_erase_range

.. doxygenfunction:: scc_vec_clear

Queries
-------

.. doxygenfunction:: scc_vec_size

.. doxygenfunction:: scc_vec_capacity

.. doxygenfunction:: scc_vec_empty

Copying
-------

.. doxygendefine:: scc_vec_clone

Iteration
---------

.. doxygendefine:: scc_vec_foreach

.. doxygendefine:: scc_vec_foreach_reversed

.. doxygendefine:: scc_vec_foreach_by

.. doxygendefine:: scc_vec_foreach_reversed_by
