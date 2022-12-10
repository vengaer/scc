#ifndef SCC_RBMAP_H
#define SCC_RBMAP_H

#include "rbtree.h"

#include <stddef.h>

//? .. _scc_rbmap_impl_pair:
//? .. c:macro:: scc_rbmap_impl_pair(keytype, valuetype)
//?
//?     Expands to a struct appropriate for storing pairs of
//?     the supplied key-value types
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The key type of the hashmap
//?     :param valuetype: The value type of the hashmap
#define scc_rbmap_impl_pair(keytype, valuetype)                                             \
    struct { keytype rm_key; valuetype rm_value; }

//! .. c:macro:: scc_rbmap(keytype, valuetype)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to an rbmap mapping instances of the specified
//!     key type to value type instances
//!
//!     :param keytype: The key type to store in the map
//!     :param valuetype: The value type to storre in the map
#define scc_rbmap(keytype, valuetype)                                                       \
    scc_rbmap_impl_pair(keytype, valuetype) *

//! .. _scc_rbmap_iter:
//! .. c:macro:: scc_rbmap_iter(keytype, valuetype)
//!
//!     Expands to an opaque pointer suitable for passing as the
//!     iter parameter to :ref:`scc_rbmap_foreach <scc_rbmap_foreach>` and
//!     :ref:`scc_rbmap_foreach_reversed <scc_rbmap_foreach_reversed>`.
//!
//!     :param keytype: The key type stored in the map that is to be iterated over
//!     :param valuetype: The value type stored in the map that is to be iterated over
#define scc_rbmap_iter(keytype, valuetype)                                                  \
    struct { keytype const key; valuetype value; } *

//? .. c:macro:: scc_rbmnode_impl_layout(keytype, valuetype)
//?
//?     Actual layout of the nodes in an rbmap providing
//?     a mapping from :code:`keytype` to :code:`valuetype`
//?     instances
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys to be stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?
//?     .. c:var:: struct scc_rbnode_base *rn_left
//?
//?         See :ref:`rn_left <struct_scc_rbnode_base_rn_left>`.
//?
//?     .. c:var:: struct scc_rbnode_base *rn_right
//?
//?         See :ref:`rn_right <struct_scc_rbnode_base_rn_right>`.
//?
//?     .. c:var:: enum scc_rbcolor rn_color
//?
//?         See :ref:`rn_color <enum_scc_rbcolor_rn_color>`.
//?
//?     .. c:var:: unsigned char rn_flags
//?
//?         See :ref:`rn_flags <unsigned_char_rn_flags>`.
//?
//?     .. _unsigned_char_rn_bkoff:
//?     .. c:var:: unsigned char rn_bkoff
//?
//?         Field storing the number of padding bytes between
//?         :ref:`rn_flags <unsigned_char_rn_flags>` and
//?         :ref:`rn_pair <rbm_kvpair_rn_pair>`.
//?
//?     .. _rbm_kvpair_rn_pair:
//?     .. c:var:: rbm_kvpair rn_pair
//?
//?         The key-value pair stored in the node
#define scc_rbmnode_impl_layout(keytype, valuetype) \
    struct {                                                                                \
        struct scc_rbnode_base *rn_left;                                                    \
        struct scc_rbnode_base *rn_right;                                                   \
        enum scc_rbcolor rn_color;                                                          \
        unsigned char rn_flags;                                                             \
        unsigned char rn_bkoff;                                                             \
        scc_rbmap_impl_pair(keytype, valuetype) rn_pair;                                    \
    }

//? .. _scc_rbmap_impl_layout:
//? .. c:macro:: scc_rbmap_impl_layout(keytype, valuetype)
//?
//?     Actual layout of the nodes in an rbmap
//?     mapping :code:`keytype` to :code:`valuetype` instances
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys to be stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?
//?     .. c:var:: unsigned short const rm_dataoff
//?
//?         See :ref:`rb_dataoff <unsigned_short_const_rb_dataoff>`.
//?
//?     .. c:var:: size_t rm_size
//?
//?         See :ref:`rb_size <size_t_rb_size>`.
//?
//?     .. c:var:: scc_rbcompare rm_compare
//?
//?         See :ref:`rb_compare <scc_rbcompare_rb_compare>`.
//?
//?     .. c:var:: struct scc_arena rm_arena
//?
//?         See :ref:`rb_arena <struct_scc_arena_rb_arena>`.
//?
//?     .. c:var:: struct scc_rbsentinel rm_sentinel
//?
//?         See :ref:`rb_sentinel <struct_scc_rbsentinel_rb_sentinel>`.
//?
//?     .. _unsigned_char_rm_fwoff:
//?     .. c:var:: unsigned char rm_fwoff
//?
//?         See :ref:`rb_fwoff <unsigned_char_rb_fwoff>`.
//?
//?     .. _unsigned_char_rm_bkoff:
//?     .. c:var:: unsigned char rm_bkoff
//?
//?         Number of padding bytes between :ref:`rm_fwoff <unsigned_char_rm_fwoff>`
//?         and :ref:`rm_curr <type_rm_curr>`
//?
//?     .. _type_rm_curr:
//?     .. c:var:: type rm_curr
//?
//?         Used for temporary storage for allowing operations on rvalues. Any
//?         pairs to be inserted, removed or searched for are temporarily
//?         stored here
#define scc_rbmap_impl_layout(keytype, valuetype)                                           \
    struct {                                                                                \
        unsigned short const rm_dataoff;                                                    \
        size_t rm_size;                                                                     \
        scc_rbcompare rm_compare;                                                           \
        struct scc_arena rm_arena;                                                          \
        struct scc_rbsentinel rm_sentinel;                                                  \
        unsigned char rm_fwoff;                                                             \
        unsigned char rm_bkoff;                                                             \
        scc_rbmap_impl_pair(keytype, valuetype) rm_curr;                                    \
    }

//! .. _scc_rbmap_new:
//! .. c:function:: void *scc_rbmap_new(keytype, valuetype, scc_rbcompare compare)
//!
//!     Instantiate an rbmap mapping :code:`keytype` to :code:`valuetype` instances
//!     and using :code:`compare` for comparison.
//!
//!     The call cannot fail.
//!
//!     :param keytype: The type of the keys to be stored in the map
//!     :param valuetype: The type of the values to be stored in the map
//!     :param compare: Pointer to the comparison function to use
//!     :returns: An opaque pointer to a rbmap allocated in the frame of the calling function
#define scc_rbmap_new(keytype, valuetype, compare)                                          \
    scc_rbtree_impl_new(&(scc_rbmap_impl_layout(keytype, valuetype)) {                      \
            .rm_dataoff = offsetof(scc_rbmnode_impl_layout(keytype, valuetype), rn_pair),   \
            .rm_compare = compare,                                                          \
            .rm_arena = scc_arena_new(scc_rbmnode_impl_layout(keytype, valuetype)),         \
        },                                                                                  \
        offsetof(scc_rbmap_impl_layout(keytype, valuetype), rm_curr)                        \
   )

//! .. c:function:: size_t scc_rbmap_size(void const *map)
//!
//!     Query the size of the given rbmap
//!
//!     :param map: rbmap handle
//!     :returns: Size of the given rbmap
inline size_t scc_rbmap_size(void const *map) {
    return scc_rbtree_size(map);
}

//! .. c:function:: _Bool scc_rbmap_empty(void const *map)
//!
//!     Determine whether the given rbmap is empty
//!
//!     :param map: rbmap handle
//!     :returns: :code:`true` if the map is empty, otherwise :code:`false`.
inline _Bool scc_rbmap_empty(void const *map) {
    return !scc_rbmap_size(map);
}

//! .. c:function:: void scc_rbmap_free(void *map)
//!
//!     Reclaim memory allocated for :c:expr:`map`. The parameter must
//!     refer to a valid rbmap returned by :ref:`scc_rbmap_new <scc_rbmap_new>`
//!
//!     :param map: rbmap handle
inline void scc_rbmap_free(void *map) {
    scc_rbtree_free(map);
}

//? .. c:function:: _Bool scc_rbmap_impl_insert(void *rbmapaddr, size_t elemsize)
//?
//?     Internal insertion function. Attempt to insert the value stored at
//?     :c:texpr:`*(void **)rbmapaddr` in the map. If the key is already present,
//?     its corresponding value is updated
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbmapaddr: Address of the rbmap handle
//?     :param valsize: Size of the values stored in the map
//?     :param valoff: Offset of the value in the key-value pair
//?     :returns: A :code:`_Bool` indicating whether insertion took place
//?     :retval true: New key-value pair inserted
//?     :retval true: Value of preexisting key-value pair updated
//?     :retval false: Memory allocation failure
_Bool scc_rbmap_impl_insert(void *rbmapaddr, size_t elemsize, size_t valoff);

//! .. c:function:: _Bool scc_rbmap_insert(void *mapaddr, keytype key, valuetype value)
//!
//!     Insert the given key-value pair into the rbtree. If the key is already present
//!     in the map, its corresponding value is updated to the given one.
//!
//!     Neither the :code:`key` nor :code:`value` parameters must necessarily be the
//!     same type as those with which the rbmap was instantiated. If they are not, they
//!     are subject to implicit conversion.
//!
//!     :param mapaddr: Address of the rbmap handle
//!     :param key: The key to insert in the map
//!     :param value: The value to insert in the map
//!     :returns: A :code:`_Bool` indicating whether the insertion took place
//?     :retval true: New key-value pair inserted
//?     :retval true: Value of preexisting key-value pair updated
//?     :retval false: Memory allocation failure
//!
//!     .. code-block:: C
//!         :caption: Insert the pair {1, 2}, {2, 4} and {3, 6} in an rbmap
//!
//!         extern int compare(void const *l, void const *r);
//!
//!         scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);
//!
//!         for(int i = 1; i < 4; ++i) {
//!             if(!scc_rbmap_insert(&rbmap, i, 2 * i)) {
//!                 fprintf(stderr, "Could not insert {%d, %d}\n", i, 2 * i);
//!             }
//!         }
//!
//!         /* Use rbmap */
//!
//!         scc_rbmap_free(rbmap);
#define scc_rbmap_insert(mapaddr, key, value)                                               \
    scc_rbmap_impl_insert((                                                                 \
            (*(mapaddr))->rm_key = (key),                                                   \
            (*(mapaddr))->rm_value = (value),                                               \
            (mapaddr)                                                                       \
        ),                                                                                  \
        sizeof(*(mapaddr)),                                                                 \
        ((unsigned char const *)&(*(mapaddr))->rm_value -                                   \
            (unsigned char const *)&(*(mapaddr))->rm_key)                                   \
    )

//? .. c:function:: void *scc_rbmap_impl_find(void *map, size_t valoff)
//?
//?     Internal search function. Attempts to find the key stored at
//?     :code:`map` in the rbmap and, if found, returns the address of
//?     the corresponding value
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: rbmap handle
//?     :param valoff: Offset between key and value in the internal pair
//?     :returns: A pointer to the value corresponding to the matching key in the map,
//?               or :code:`NULL` if no such pair is found
void *scc_rbmap_impl_find(void *map, size_t valoff);

//! .. c:function:: void const *scc_rbmap_find(void *map, keytype key)
//!
//!     Search for the given key in the map and, if found, return the address of
//!     its corresponding value
//!
//!     The :code:`key` parameter must not necessarily be the same type as the one
//!     with which the rbmap was instantiated. If it is not, it is implicitly
//!     converted to the type stored in the map.
//!
//!     :param map: rbmap handle
//!     :param key: The key to search for
//!     :returns: A pointer to the value corresponding to the matching key in the map,
//!               or :code:`NULL` if no such pair is found
#define scc_rbmap_find(map, key)                                                            \
    scc_rbmap_impl_find(                                                                    \
        ((map)->rm_key = (key), (map)),                                                     \
        ((unsigned char const *)&(map)->rm_value - (unsigned char const *)&(map)->rm_key)   \
    )

//! .. c:function:: _Bool scc_rbmap_remove(void *map, keytype key)
//!
//!     Find and remove the pair identified by the given key
//!
//!     The :code:`key` parameter must not necessarily be the same type as the
//!     one with which the rbmap was instantiated. If it is not, the value is
//!     implicitly converted to the type stored in the map.
//!
//!     :param map: rbmap handle
//!     :param key: Key idenfiying the pair to be removed
//!     :returns: :code:`true` if the key-value pair was removed, :code:`false` if no
//!                such pair was found
#define scc_rbmap_remove(map, key)                                                          \
    scc_rbtree_impl_remove(                                                                 \
        ((map)->rm_key = (key), (map)),                                                     \
        sizeof(*(map))                                                                      \
    )

//? .. c:function:: void *scc_rbmap_impl_leftmost_pair(void *map)
//?
//?     Find and return the address of the pair stored in the leftmost node
//?     in the given map
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: rbmap handle
//?     :returns:   Address of the pair in the leftmost node in the map
void *scc_rbmap_impl_leftmost_pair(void *map);

//? .. c:function:: void *scc_rbmap_impl_rightmost_pair(void *map)
//?
//?     Find and return the address of the pair stored in the rightmost
//?     node in the given map
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: rbmap handle
//?     :returns:   Address of the pair in the rightmost node in the map
void *scc_rbmap_impl_rightmost_pair(void *map);

//? .. c:function:: void *scc_rbmap_impl_successor(void *iter)
//?
//?     Find and return the address of the in-order successor of the given address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param iter: Address of the :ref:`rn_pair <rbm_kvpair_rn_pair>` field in
//?                  the currently traversed node
//?     :returns:    Address of the in-order successor of the given node
void *scc_rbmap_impl_successor(void *iter);

//? .. c:function:: void *scc_rbmap_impl_predecessor(void *iter)
//?
//?     Find and return the address of the in-order predecessor of the given address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param iter: Address of the :ref:`rn_pair <rbm_kvpair_rn_pair>` field in
//?                  the currently traversed node
//?     :returns:    Address of the in-order predecessor of the given node
void *scc_rbmap_impl_predecessor(void *iter);

//? .. c:function:: void const *scc_rbmap_impl_iterstop(void const *map)
//?
//?     Compute address to be used as stop marker in :ref:`scc_rbmap_foreach <scc_rbmap_foreach>`
//?     and :ref:`scc_rbmap_foreach_reversed <scc_rbmap_foreach_reversed>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: Map handle
//?     :returns:   An address within the rbmap struct to be used
//?                 as stop address for the iteration
inline void const *scc_rbmap_impl_iterstop(void const *map) {
    return scc_rbtree_impl_iterstop(map);
}

//! .. _scc_rbmap_foreach:
//! .. c:macro:: scc_rbmap_foreach(iter, map)
//!
//!     In-order iteration of the nodes in the rbmap. The macro results in a scope
//!     executed once for each key-value pair in the tree with :code:`iter` exposing
//!     the key and value as :c:texpr:`iter->key` and :c:texpr:`iter->value`, respectively.
//!
//!     :param iter:  An instance obtained by invoking :ref:`scc_rbmap_iter <scc_rbmap_iter>` with the
//!                   appropriate key and value types
//!     :param rbmap: rbmap handle
//!
//!     .. code-block:: C
//!         :caption: Iterate over and duplicate each value in an rbmap mapping
//!                   ints to ints
//!
//!         extern scc_rbmap(int, int) rbmap;
//!
//!         // Iterator
//!         scc_rbmap_iter(int, int) iter;
//!
//!         scc_rbmap_foreach(iter, rbmap) {
//!             printf("Value of %d %d -> %d\n", iter->key, iter->value, iter->value * 2);
//!             iter->value *= 2;
//!         }
#define scc_rbmap_foreach(iter, rbmap)                                                      \
    for(void const *scc_pp_cat_expand(scc_rbmap_end_,__LINE__) =                            \
            (iter = scc_rbmap_impl_leftmost_pair(rbmap),                                    \
                scc_rbmap_impl_iterstop(rbmap));                                            \
        iter != scc_pp_cat_expand(scc_rbmap_end_,__LINE__);                                 \
        iter = scc_rbmap_impl_successor(iter))

//! .. _scc_rbmap_foreach_reversed:
//! .. c:macro:: scc_rbmap_foreach_reversed(iter, rbmap)
//!
//!     Reversed in-order iteration of the nodes in the rbmap.
//!
//!     .. seealso::
//!
//!         :ref:`scc_rbmap_foreach <scc_rbmap_foreach>`
//!
//!     :param iter:  An instance obtained by invoking :ref:`scc_rbmap_iter <scc_rbmap_iter>` with the
//!                   appropriate key and value types
//!     :param rbmap: rbmap handle
#define scc_rbmap_foreach_reversed(iter, rbmap)                                             \
    for(void const *scc_pp_cat_expand(scc_rbmap_end_,__LINE__) =                            \
            (iter = scc_rbmap_impl_rightmost_pair(rbmap),                                   \
                scc_rbmap_impl_iterstop(rbmap));                                            \
        iter != scc_pp_cat_expand(scc_rbmap_end_,__LINE__);                                 \
        iter = scc_rbmap_impl_predecessor(iter))

#endif /* SCC_RBMAP_H */
