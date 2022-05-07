===========
Collections
===========

SCC provides a set of efficient, fully generic C collections. The implementations are entirely self-contained and depend neither on each other [#f1]_ nor external libraries.

.. [#f1] The adapter collections, currently comprising only :c:texpr:`scc_stack`, are a notable exception to this. See :ref:`Adapter Types <adapter_types>` for more info.

Hash Collections
================

.. toctree::
    hashmap/index
    hashtab/index

Array-Like Collections
======================

.. toctree::

    svec/index
    vec/index

Trees
=====

.. toctree::

    rbtree/index

.. _adapter_types:

Adapter Types
=============

..
  _TODO


.. toctree::

    stack/index
