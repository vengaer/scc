=========
scc_btmap
=========

``scc_btmap`` provides a mapping of unique keys to values much like C++'s `std::map <https://en.cppreference.com/w/cpp/container/map>`_. A notable difference between the two is that while the latter is usually implemented as a red-black tree, ``scc_btmap`` is built on top of a B-tree, making it more similar to Rust's `BTreeMap <https://doc.rust-lang.org/std/collections/struct.BTreeMap.html>`_.

While B-trees may contain duplicate keys, ``scc_btmap`` does *not* allow this. Instead, inserting a key-value pair whose key is already present in the map will result in the new value overriding whichever one was previously associated with the key in the map. This behavior is identical to that of the aforementioned ``BTreeMap`` but differs notably from that of
``scc_btree``.

.. toctree::

    reference
