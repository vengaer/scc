***********
Hash Table
***********

The SCC hash table implementation is accessible through the primitives in the `scc_hashtab.h` header. The API works on an opaque pointer created via `scc_hashtab(type)` where `type` is the type the to be stored in the table.

The implementation is currently supported only on systems running on an AVX2-compatible processor abiding by the SystemV calling convention.

.. toctree::
    :maxdepth: 3
    :hidden:

    types
