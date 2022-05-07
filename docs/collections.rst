===========
Collections
===========

SCC provides a set of efficient, fully generic C collections. The implementations are entirely self-contained and depend neither on each other [#f1]_ nor external libraries.

.. [#f1] The adapter collections, currently comprising only :c:texpr:`scc_stack`, are a notable exception to this. See :ref:`Adapter Types` for more info.

Adapter Types
=============

..
  _TODO


.. toctree::
    :maxdepth: 3

    hashmap
    hashtab
    rbtree
    stack
    svec
    vec
