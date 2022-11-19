#ifndef SCC_BTMAP_H
#define SCC_BTMAP_H

#include "scc_btree.h"

#ifndef SCC_BTMAP_DEFAULT_ORDER
//! .. _scc_btree_default_order:
//! .. c:macro:: SCC_BTMAP_DEFAULT_ORDER
//!
//!     Default :ref:`order <btree_order>` of B-treemaps
//!     instantiated using :ref:`scc_btmap_new <scc_btmap_new>`.
//!
//!     The value may be overridden by defining it before including
//!     the header. B-treemaps of a specific order may be instantiated
//!     using :ref:`scc_btmap_with_order <scc_btmap_with_order>`
//!
//!     Must be greater than 2. The implementation also
//!     requires that it is smaller than :c:texpr:`SIZE_MAX / 2`, but
//!     chosing values even remotely that size is ill-adviced.
#define SCC_BTMAP_DEFAULT_ORDER SCC_BTREE_DEFAULT_ORDER
#endif /* SCC_BTMAP_DEFAULT_ORDER */

//? .. _scc_btmap_impl_pair:
//? .. c:macro:: scc_btmap_impl_pair(keytype, valuetype)
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
#define scc_btmap_impl_pair(keytype, valuetype)                                                                 \
    struct { keytype btm_key; valuetype btm_value; }

//! .. c:macro:: scc_btmap(keytype, valuetype)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to a B-treemap mapping instances of the specified
//!     key type to value type instances
//!
//!     :param keytype: The key type to store in the map
//!     :param valuetype: The value type to storre in the map
#define scc_btmap(keytype, valuetype)                                                                           \
    scc_btmap_impl_pair(keytype, valuetype) *

//? .. c:macro:: scc_btmnode_base
//?
//?     Alias for scc_btnode_base to provide a unified
//?     naming scheme in the btmap while avoiding code
//?     duplication
//?
//?     .. note::
//?
//?         Internal use only
#define scc_btmnode_base scc_btnode_base

//? .. _scc_btmap_base:
//? .. c:struct:: scc_btmap_base
//?
//?     Base structure of the B-treemap
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _unsigned_short_const_btm_order:
//?     .. c:var:: unsigned short const btm_order
//?
//?         Order of the B-treemap
//?
//?     .. _unsigned_short_const_btm_keyoff:
//?     .. c:var:: unsigned short const btm_keyoff
//?
//?         Offset of each node's key array relative its base address
//?
//?     .. _unsigned_short_const_btm_valoff:
//?     .. c:var:: unsigned short const btm_valoff
//?
//?         Offset of each node's value array relative its base address
//?
//?     .. _unsigned_short_const_btm_linkoff:
//?     .. c:var:: unsigned short const btm_linkoff
//?
//?         Offset of echo node's array of link pointers
//?
//?     .. _size_t_btm_size:
//?     .. c:var:: size_t btm_size
//?
//?         Size of the B-treemap
//?
//?     .. _struct_scc_btmnode_base_btm_root:
//?     .. c:var:: struct scc_btmnode_base *btm_root
//?
//?         Address of the root node
//?
//?     .. _struct_scc_arena_btm_arena:
//?     .. c:var:: struct scc_arena btm_arena
//?
//?         Node allocator
//?
//?     .. _scc_bcompare_btm_compare:
//?     .. c:var:: scc_bcompare btm_compare
//?
//?         Pointer to comparison function
//?
//?     .. _unsigned_char_btm_fwoff:
//?     .. c:var:: unsigned char btm_fwoff
//?
//?         Number of padding bytes between the field itself and
//?         :ref:`btm_curr <kvpair_btm_curr>`
//?
//?     .. c:var:: unsigned char btm_data[]
//?
//?         FAM used for storing :ref:`bt_curr <type_bt_curr>`
struct scc_btmap_base {
    unsigned short const btm_order;
    unsigned short const btm_keyoff;
    unsigned short const btm_valoff;
    unsigned short const btm_linkoff;
    size_t btm_size;
    struct scc_btmnode_base *btm_root;
    struct scc_arena btm_arena;
    scc_bcompare btm_compare;
    unsigned char btm_fwoff;
    unsigned char btm_data[];
};

//? .. _scc_btmnode_impl_layout:
//? .. c:macro:: scc_btmnode_impl_layout(keytype, valuetype, order)
//?
//?     Actual layout of the nodes in a B-treemap of order
//?     :code:`order` storing keys and values of the provided types
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys to be stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?     :param order: The order to the B-tree
//?
//?     .. c:var:: unsigned char btm_flags
//?
//?         See :ref:`bt_flags <unsigned_char_bt_flags>`.
//?
//?     .. c:var:: unsigned short btm_nkeys
//?
//?         See :ref:`bt_nkeys <unsigned_short_bt_nkeys>`.
//?
//?     .. _type_btm_keys:
//?     .. c:var:: type btm_keys[order - 1u]
//?
//?         Array storing the keys contained in the node. At any
//?         given time, the :code:`btm_nkeys` first nodes of the
//?         array are in use
//?
//?     .. _type_btm_vals:
//?     .. c:var:: type btm_vals[order - 1u]
//?
//?         Array storing the values contained in the node. At any
//?         given time, the :code:`btm_nkeys` first nodes of the
//?         array are in use
//?
//?     .. _struct_scc_btmnode_base_btm_links:
//?     .. c:var:: struct scc_btmnode_base *btm_links[order]
//?
//?         Links to other nodes in the B-treemap. At most
//?         the :c:expr:`btm_nkeys + 1u` first nodes are used.
#define scc_btmnode_impl_layout(keytype, valuetype, order)                                                          \
    struct {                                                                                                        \
        unsigned char btm_flags;                                                                                    \
        unsigned short btm_nkeys;                                                                                   \
        keytype btm_keys[(order) - 1u];                                                                             \
        valuetype btm_vals[(order) - 1u];                                                                           \
        struct scc_btmnode_base *btm_links[order];                                                                  \
    }

//? .. c:macro:: scc_btmap_impl_layout(keytype, valuetype, order)
//?
//?     Actual layout of a B-treemap storing instances of the given
//?     key and value types
//?
//?     The :code:`btm_order` through :code:`btm_fwoff` members are identical
//?     to the ones in :ref:`struct scc_btmap_base <scc_btmap_base>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys to be stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?     :param order: The order to the B-tree
//?
//?     .. c:var:: unsigned short const btm_order
//?
//?         See :ref:`btm_order <unsigned_short_const_btm_order>`
//?
//?     .. c:var:: unsigned short const btm_keyoff
//?
//?         See :ref:`btm_keyoff <unsigned_short_const_btm_keyoff>`
//?
//?     .. c:var:: unsigned short const btm_valoff
//?
//?         See :ref:`btm_valoff <unsigned_short_const_btm_valoff>`
//?
//?     .. c:var:: unsigned short const btm_linkoff
//?
//?         See :ref:`btm_linkoff <unsigned_short_const_btm_linkoff>`
//?
//?     .. c:var:: size_t btm_size
//?
//?         See :ref:`btm_size <size_t_btm_size>`
//?
//?     .. c:var:: struct scc_btmnode_base *btm_root
//?
//?         See :ref:`btm_root <struct_scc_btmnode_base_btm_root>`
//?
//?     .. c:var:: struct scc_arena btm_arena
//?
//?         See :ref:`btm_arena <struct_scc_arena_btm_arena>`
//?
//?     .. c:var:: scc_bcompare btm_compare
//?
//?         See :ref:`btm_compare <scc_bcompare_btm_compare>`
//?
//?     .. c:var:: unsigned char btm_fwoff
//?
//?         See :ref:`btm_fwoff <unsigned_char_btm_fwoff>`
//?
//?     .. _unsigned_char_btm_bkoff:
//?     .. c:var:: unsigned char btm_bkoff
//?
//?         Number of padding bytes between :ref:`btm_fwoff <unsigned_char_btm_fwoff>`
//?         and :ref:`btm_curr <type_btm_curr>`
//?
//?     .. _kvpair_btm_curr:
//?     .. c:var:: kvpair btm_curr
//?
//?         Used for temporary storage for allowing operations on rvalues. Any
//?         elements to be inserted, removed or searched for are first written
//?         here
//?
//?     .. c:var:: struct scc_btmnode btm_rootmem
//?
//?         Automatically allocated memory for the root node to ensure that handle
//?         returned by initialization functions such as
//?         :ref:`scc_btmap_new <scc_btmap_new>` do not have to be :code:`NULL`
//?         checked.
#define scc_btmap_impl_layout(keytype, valuetype, order)                                                                \
    struct {                                                                                                            \
        unsigned short const btm_order;                                                                                 \
        unsigned short const btm_keyoff;                                                                                \
        unsigned short const btm_valoff;                                                                                \
        unsigned short const btm_linkoff;                                                                               \
        size_t btm_size;                                                                                                \
        struct scc_btmnode_base *btm_root;                                                                              \
        struct scc_arena btm_arena;                                                                                     \
        scc_bcompare btm_compare;                                                                                       \
        unsigned char btm_fwoff;                                                                                        \
        unsigned char btm_bkoff;                                                                                        \
        scc_btmap_impl_pair(keytype, valuetype) btm_curr;                                                               \
        scc_btmnode_impl_layout(keytype, valuetype,order) btm_rootmem;                                                  \
    }

//! .. _scc_btmap_with_order:
//! .. c:function:: void *scc_btmap_with_order(keytype, valuetype, scc_bcompare compare, unsigned order)
//!
//!     Instantiate a B-treemap of :ref:`order <btree_order>` :code:`order`, storing instances
//!     of the given key and value types, the former being compared using the supplied :code:`compare` function.
//!
//!     The map is constructed in the frame of the calling function. For more information, see
//!     :ref:`Scope and Lifetimes <scope_and_lifetimes>`.
//!
//!     :param keytype: The type of the keys to be stored in the B-tree
//!     :param valuetype: The type of the values to be stored in the B-tree
//!     :param compare: Pointer to the comparison function to use
//!     :param order: The :ref:`order <btree_order>` of the B-treemap. Must be a value
//!                   greater than 2
//!     :returns: An opaque pointer to a B-tree allocated in the frame of the calling function,
//!               or NULL if the order is invalid
#define scc_btmap_with_order(keytype, valuetype, compare, order)                                                        \
    scc_btree_impl_with_order(&(scc_btmap_impl_layout(keytype, valuetype, order)) {                                     \
            .btm_order = order,                                                                                         \
            .btm_keyoff = offsetof(scc_btmnode_impl_layout(keytype, valuetype, order), btm_keys),                       \
            .btm_valoff = offsetof(scc_btmnode_impl_layout(keytype, valuetype, order), btm_vals),                       \
            .btm_linkoff = offsetof(scc_btmnode_impl_layout(keytype, valuetype, order), btm_links),                     \
            .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, order)),                             \
            .btm_compare = compare                                                                                      \
        },                                                                                                              \
        offsetof(scc_btmap_impl_layout(keytype, valuetype, order), btm_curr),                                           \
        offsetof(scc_btmap_impl_layout(keytype, valuetype, order), btm_rootmem)                                         \
    )

//! .. _scc_btmap_new:
//! .. c:function:: void *scc_btmap_new(keytype, valuetype, scc_bcompare compare)
//!
//!     Instantiate a B-treemap of the configured :ref:`default order <scc_btree_default_order>`
//!     storing instances of the given key and value types and using :code:`compare` for comparison.
//!
//!     Calling :code:`scc_btmap_new` is entirely equivalent to calling
//!     :code:`scc_btmap_with_order` with the :code:`order` parameter set to
//!     :code:`SCC_BTMAP_DEFAULT_ORDER`. See :ref:`scc_btmap_with_order <scc_btmap_with_order>`
//!     for more information.
//!
//!     The call cannot fail.
//!
//!     :param keytype: The type of the keys to be stored in the map
//!     :param valuetype: The type of the values to be stored in the map
//!     :param compare: Pointer to the comparison function to use
//!     :returns: An opaque pointer to a B-tree allocated in the frame of the calling function
#define scc_btmap_new(keytype, valuetype, compare)                                                                      \
    scc_btree_impl_new(&(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)) {                          \
            .btm_order = SCC_BTMAP_DEFAULT_ORDER,                                                                       \
            .btm_keyoff = offsetof(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER), btm_keys),     \
            .btm_valoff = offsetof(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER), btm_vals),     \
            .btm_linkoff = offsetof(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER), btm_links),   \
            .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)),           \
            .btm_compare = compare                                                                                      \
        },                                                                                                              \
        offsetof(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER), btm_curr),                         \
        offsetof(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER), btm_rootmem)                       \
    )

//! .. c:function:: void scc_btmap_free(void *btmap)
//!
//!     Reclaim memory allocated for :c:expr:`btmap`. The parameter must
//!     refer to a valid B-treemap returned by :ref:`scc_btmap_new <scc_btmap_new>` or
//!     :ref:`scc_btmap_with_order <scc_btmap_with_order>`.
//!
//!     :param btmap: B-treemap handle
inline void scc_btmap_free(void *btmap) {
    scc_btree_free(btmap);
}

#endif /* SCC_BTMAP_H */
