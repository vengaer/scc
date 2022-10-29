#ifndef SCC_RBMAP_H
#define SCC_RBMAP_H

#include "scc_rbtree.h"

#include <stddef.h>

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
//?         :ref:`rn_pair <type_rn_pair>`.
//?
//?     .. _type_rn_pair:
//?     .. c:var:: type rn_pair
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

//! .. c:function:: _Bool scc_rbmap_insert(void *mapaddr, keytype key, valuetype value)
//!
//!     Insert the given key-value pair into the rbtree.
//!
//!     Neither the :code:`key` nor :code:`value` parameters must necessarily be the
//!     same type as those with which the rbmap was instantiated. If they are not, they
//!     are subject to implicit conversion.
//!
//!     :param mapaddr: Address of the rbmap handle
//!     :param key: The key to insert in the map
//!     :param value: The value to insert in the map
//!     :returns: A :code:`_Bool` indicating whether the insertion took place
//!     :retval true: The key-value pair was inserted
//!     :retval false: The key was already in the map, or memory allocation failure
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
    scc_rbtree_impl_insert((                                                                \
            (*(mapaddr))->rm_key = (key),                                                   \
            (*(mapaddr))->rm_value = (value),                                               \
            (mapaddr)                                                                       \
        ),                                                                                  \
        sizeof(**(mapaddr))                                                                 \
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

#endif /* SCC_RBMAP_H */
