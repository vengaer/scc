====================================
Hash Collection Collision Resolution
====================================

Both :ref:`scc_hashtab <scc_hashtab>` and :ref:`scc_hashmap <scc_hashmap>` rely on `open addressing <https://en.wikipedia.org/wiki/Open_addressing>`_ for resolving hashing conflicts. Put simply, the process of searching for an element in a hash collection using open addressing relies on computing the hash of the value to be found, using said hash to compute a slot of the table which is used as the base for a separate probe. The ``scc`` hash collections use linear probing to make as good a use of modern hardware as possible.

Default Hash Function
=====================

``scc`` uses the `Fowler-Noll-Vo alternative <https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function#FNV-1a_hash>`_ hash function by default which provides good so called avalanche characteristics (i.e. small changes to the input resulting in drastically different hashes). While the default choice hash function is theoretically excellent, improved performance may sometimes be obtained by using a less computationally intensive one. As an example, `LLVM's libc++ <https://libcxx.llvm.org/>`_ uses the significantly faster identity function for hashing integers.

Probing Implementation
======================

A naïve implementation of the linear probing used by ``scc`` would simply start at the computed slot and iterate over each subsequent slot, calling ``eq``, the equality function assigned to the collection at creation, on each slot. While this works, it would be unnecessarily slow. Not only does each ``eq`` call come with the overhead of a normal function call, the fact that it is performed through a function pointer may also inhibit both processor and compiler optimizations. To limit this overhead, ``scc`` stores part of the hash of each element computed when it was inserted. On `SIMD <https://en.wikipedia.org/wiki/Single_instruction,_multiple_data>`_-capable processors, this metadata may be searched in parallel. ``scc`` currently implements support for this on processors supporting the `AVX2 <https://en.wikipedia.org/wiki/Advanced_Vector_Extensions>`_ instruction set architecture.

