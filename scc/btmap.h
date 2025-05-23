#ifndef SCC_BTMAP_H
#define SCC_BTMAP_H

#include "arena.h"
#include "btree.h"
#include "mem.h"

#include <stddef.h>

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

//? .. c:macro:: scc_btmap_impl_pair_valoff(keytype, valuetype)
//?
//?     Compute offset of value field in a generic pair.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The key type of the hashmap
//?     :param valuetype: The value type of the hashmap
#define scc_btmap_impl_pair_valoff(keytype, valuetype)                                                          \
    scc_align(sizeof(keytype), scc_alignof(valuetype))

//! .. c:macro:: scc_btmap(keytype, valuetype)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to a B-tree map mapping instances of the specified
//!     key type to value type instances
//!
//!     :param keytype: The key type to store in the map
//!     :param valuetype: The value type to storre in the map
#define scc_btmap(keytype, valuetype)                                                                           \
    scc_btmap_impl_pair(keytype, valuetype) *

#ifndef SCC_BTMAP_DEFAULT_ORDER
//! .. _scc_btmap_default_order:
//! .. c:macro:: SCC_BTMAP_DEFAULT_ORDER
//!
//!     Default :ref:`order <btree_order>` of B-tree maps
//!     instantiated using :ref:`scc_btmap_new <scc_btmap_new>`.
//!
//!     The value may be overridden by defining it before including
//!     the header. B-tree maps of a specific order may be instantiated
//!     using :ref:`scc_btmap_with_order <scc_btmap_with_order>`
//!
//!     Must be greater than 2. The implementation also
//!     requires that it is smaller than :c:texpr:`SIZE_MAX / 2`, but
//!     chosing values even remotely that size is ill-adviced.
#define SCC_BTMAP_DEFAULT_ORDER SCC_BTREE_DEFAULT_ORDER
#endif /* SCC_BTMAP_DEFAULT_ORDER */

//? .. _scc_btmnode_base:
//? .. c:struct:: scc_btmnode_base
//?
//?     Generic B-tree map node structure
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _unsigned_char_btm_flags:
//?     .. c:var:: unsigned char btm_flags
//?
//?         Link flags. Used bits are as follows
//?
//?         .. list-table:: Flag Bits
//?             :header-rows: 1
//?
//?             * - Bit
//?               - Meaning
//?             * - 0x01
//?               - Node is a leaf
//?
//?     .. _unsigned_short_btm_nkeys:
//?     .. c:var:: unsigned short btm_nkeys
//?
//?         Number of keys stored in the node
//?
//?     .. c:var:: unsigned char btm_data[]
//?
//?         FAM hiding type-specific details. See
//?         :ref:`scc_btmnode_impl_layout <scc_btmnode_impl_layout>`
//?         for details.
struct scc_btmnode_base {
    unsigned char btm_flags;
    unsigned short btm_nkeys;
    unsigned char btm_data[];
};

//? .. _scc_btmap_base:
//? .. c:struct:: scc_btmap_base
//?
//?     Base structure of the B-tree map
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _unsigned_short_const_btm_order:
//?     .. c:var:: unsigned short const btm_order
//?
//?         Order of the B-tree map
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
//?     .. _unsigned_short_const_btm_keysize:
//?     .. c:var:: unsigned short const btm_keysize
//?
//?         Size of the keys in the ``btmap``
//?
//?     .. _unsigned_short_const_btm_valsize:
//?     .. c:var:: unsigned short const btm_valsize
//?
//?         Size of the values in the ``btmap``
//?
//?     .. _size_t_btm_size:
//?     .. c:var:: size_t btm_size
//?
//?         Size of the B-tree map
//?
//?     .. _struct_scc_btmnode_base_btm_root:
//?     .. c:var:: struct scc_btmnode_base *btm_root
//?
//?         Address of the root node
//?
//?     .. _scc_bcompare_btm_compare:
//?     .. c:var:: scc_bcompare btm_compare
//?
//?         Pointer to comparison function
//?
//?     .. _struct_scc_arena_btm_arena:
//?     .. c:var:: struct scc_arena btm_arena
//?
//?         Node allocator
//?
//?     .. _unsigned_char_btm_kvoff:
//?     .. c:var:: unsigned char const btm_kvoff
//?
//?         Offset of the value in the
//?         :ref:`key-value pair <scc_btmap_impl_pair>`
//?
//?     .. _unsigned_char_btm_dynalloc:
//?     .. c:var:: unsigned char btm_dynalloc
//?
//?         Set to 1 of the struct was dynamically allocated
//?
//?     .. _unsigned_char_btm_fwoff:
//?     .. c:var:: unsigned char btm_fwoff
//?
//?         Number of padding bytes between the field itself and
//?         :ref:`btm_curr <kvpair_btm_curr>`
//?
//?     .. c:var:: unsigned char btm_data[]
//?
//?         FAM used for storing :ref:`bt_curr <kvpair_btm_curr>`
struct scc_btmap_base {
    unsigned short const btm_order;
    unsigned short const btm_keyoff;
    unsigned short const btm_valoff;
    unsigned short const btm_linkoff;
    unsigned short const btm_keysize;
    unsigned short const btm_valsize;
    size_t btm_size;
    struct scc_btmnode_base *btm_root;
    scc_bcompare btm_compare;
    struct scc_arena btm_arena;
    unsigned char const btm_kvoff;
    unsigned char btm_dynalloc;
    unsigned char btm_fwoff;
    unsigned char btm_data[];
};

//? .. _scc_btmnode_impl_layout:
//? .. c:macro:: scc_btmnode_impl_layout(keytype, valuetype, order)
//?
//?     Actual layout of the nodes in a B-tree map of order
//?     :code:`order` storing keys and values of the provided types
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys to be stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?     :param order: The order to the ``btmap``
//?
//?     .. c:var:: unsigned char btm_flags
//?
//?         See :ref:`btm_flags <unsigned_char_btm_flags>`.
//?
//?     .. c:var:: unsigned short btm_nkeys
//?
//?         See :ref:`bt_mnkeys <unsigned_short_btm_nkeys>`.
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
//?         Links to other nodes in the B-tree map. At most
//?         the :c:expr:`btm_nkeys + 1u` first nodes are used.
#define scc_btmnode_impl_layout(keytype, valuetype, order)                                                          \
    struct {                                                                                                        \
        struct {                                                                                                    \
            struct {                                                                                                \
                struct {                                                                                            \
                    unsigned char btm_flags;                                                                        \
                    unsigned short btm_nkeys;                                                                       \
                } btmn0;                                                                                            \
                keytype btm_keys[(order) - 1u];                                                                     \
            } btmn1;                                                                                                \
            valuetype btm_vals[(order) - 1u];                                                                       \
        } btmn2;                                                                                                    \
        struct scc_btmnode_base *btm_links[order];                                                                  \
    }

//? .. c:macro:: scc_btmnode_impl_keyoff(keytype)
//?
//?     Compute offset of the key array.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys stored in the node
#define scc_btmnode_impl_keyoff(keytype)                                                                            \
    sizeof(                                                                                                         \
        struct {                                                                                                    \
            struct {                                                                                                \
                unsigned char btm_flags;                                                                            \
                unsigned short btm_nkeys;                                                                           \
            } btmn0;                                                                                                \
            keytype btm_keys[];                                                                                     \
        }                                                                                                           \
    )

//? .. c:macro:: scc_btmnode_impl_valoff(keytype)
//?
//?     Compute offset of the value array.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?     :param order: The order to the ``btmap``
#define scc_btmnode_impl_valoff(keytype, valuetype, order)                                                          \
    sizeof(                                                                                                         \
        struct {                                                                                                    \
            struct {                                                                                                \
                struct {                                                                                            \
                    unsigned char btm_flags;                                                                        \
                    unsigned short btm_nkeys;                                                                       \
                } btmn0;                                                                                            \
                keytype btm_keys[(order) - 1u];                                                                     \
            } btmn1;                                                                                                \
            valuetype btm_vals[];                                                                                   \
        }                                                                                                           \
    )

//? .. c:macro:: scc_btmnode_impl_linkoff(keytype)
//?
//?     Compute offset of the link array
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?     :param order: The order to the ``btmap``
#define scc_btmnode_impl_linkoff(keytype, valuetype, order)                                                         \
    sizeof(                                                                                                         \
        struct {                                                                                                    \
            struct {                                                                                                \
                struct {                                                                                            \
                    struct {                                                                                        \
                        unsigned char btm_flags;                                                                    \
                        unsigned short btm_Nkeys;                                                                   \
                    } btmn0;                                                                                        \
                    keytype btm_keys[(order) - 1u];                                                                 \
                } btmn1;                                                                                            \
                valuetype btm_vals[(order) - 1u];                                                                   \
            } btmn2;                                                                                                \
            struct scc_btmnode_base *btm_links[];                                                                   \
        }                                                                                                           \
    )

//? .. c:macro:: scc_btmap_impl_layout(keytype, valuetype, order)
//?
//?     Actual layout of a B-tree map storing instances of the given
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
//?     :param order: The order to the ``btmap``
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
//?     .. c:var:: unsigned short const btm_keysize
//?
//?         See :ref:`btm_keysize <unsigned_short_const_btm_keysize>`
//?
//?     .. c:var:: unsigned short const btm_valsize
//?
//?         See :ref:`btm_valsize <unsigned_short_const_btm_valsize>`
//?
//?     .. c:var:: size_t btm_size
//?
//?         See :ref:`btm_size <size_t_btm_size>`
//?
//?     .. c:var:: struct scc_btmnode_base *btm_root
//?
//?         See :ref:`btm_root <struct_scc_btmnode_base_btm_root>`
//?
//?     .. c:var:: scc_bcompare btm_compare
//?
//?         See :ref:`btm_compare <scc_bcompare_btm_compare>`
//?
//?     .. c:var:: struct scc_arena btm_arena
//?
//?         See :ref:`btm_arena <struct_scc_arena_btm_arena>`
//?
//?     .. c:var:: unsigned char const btm_kvoff
//?
//?         See :ref:`btm_kvoff <unsigned_char_btm_kvoff>`
//?
//?     .. c:var:: unsigned char btm_dynalloc
//?
//?         See :ref:`btm_dynalloc <unsigned_char_btm_dynalloc>`
//?
//?     .. c:var:: unsigned char btm_fwoff
//?
//?         See :ref:`btm_fwoff <unsigned_char_btm_fwoff>`
//?
//?     .. _unsigned_char_btm_bkoff:
//?     .. c:var:: unsigned char btm_bkoff
//?
//?         Number of padding bytes between :ref:`btm_fwoff <unsigned_char_btm_fwoff>`
//?         and :ref:`btm_curr <kvpair_btm_curr>`
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
        struct {                                                                                                        \
            struct {                                                                                                    \
                unsigned short const btm_order;                                                                         \
                unsigned short const btm_keyoff;                                                                        \
                unsigned short const btm_valoff;                                                                        \
                unsigned short const btm_linkoff;                                                                       \
                unsigned short const btm_keysize;                                                                       \
                unsigned short const btm_valsize;                                                                       \
                size_t btm_size;                                                                                        \
                struct scc_btmnode_base *btm_root;                                                                      \
                scc_bcompare btm_compare;                                                                               \
                struct scc_arena btm_arena;                                                                             \
                unsigned char const btm_kvoff;                                                                          \
                unsigned char btm_dynalloc;                                                                             \
                unsigned char btm_fwoff;                                                                                \
                unsigned char btm_bkoff;                                                                                \
            } btm0;                                                                                                     \
            scc_btmap_impl_pair(keytype, valuetype) btm_curr;                                                           \
        } btm1;                                                                                                         \
        scc_btmnode_impl_layout(keytype, valuetype,order) btm_rootmem;                                                  \
    }

//? .. c:macro:: scc_btmap_impl_curroff(keytype, valuetype, order)
//?
//?     Compute offset of :ref:`btm_curr <kvpair_btm_curr>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys to be stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?     :param order: The order to the ``btmap``
#define scc_btmap_impl_curroff(keytype, valuetype, order)                                                               \
    sizeof(                                                                                                             \
        struct {                                                                                                        \
            struct {                                                                                                    \
                unsigned short const btm_order;                                                                         \
                unsigned short const btm_keyoff;                                                                        \
                unsigned short const btm_valoff;                                                                        \
                unsigned short const btm_linkoff;                                                                       \
                unsigned short const btm_keysize;                                                                       \
                unsigned short const btm_valsize;                                                                       \
                size_t btm_size;                                                                                        \
                struct scc_btmnode_base *btm_root;                                                                      \
                scc_bcompare btm_compare;                                                                               \
                struct scc_arena btm_arena;                                                                             \
                unsigned char const btm_kvoff;                                                                          \
                unsigned char btm_dynalloc;                                                                             \
                unsigned char btm_fwoff;                                                                                \
                unsigned char btm_bkoff;                                                                                \
            } btmn0;                                                                                                    \
            scc_btmap_impl_pair(keytype, valuetype) btm_curr[];                                                         \
        }                                                                                                               \
    )

//? .. c:macro:: scc_btmap_impl_rootoff(keytype, valuetype, order)
//?
//?     Compute offset of automatic root node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param keytype: The type of the keys to be stored in the node
//?     :param valuetype: The type of the values to be stored in the node
//?     :param order: The order to the ``btmap``
#define scc_btmap_impl_rootoff(keytype, valuetype, order)                                                               \
    sizeof(                                                                                                             \
        struct {                                                                                                        \
            struct {                                                                                                    \
                struct {                                                                                                \
                    unsigned short const btm_order;                                                                     \
                    unsigned short const btm_keyoff;                                                                    \
                    unsigned short const btm_valoff;                                                                    \
                    unsigned short const btm_linkoff;                                                                   \
                    unsigned short const btm_keysize;                                                                   \
                    unsigned short const btm_valsize;                                                                   \
                    size_t btm_size;                                                                                    \
                    struct scc_btmnode_base *btm_root;                                                                  \
                    scc_bcompare btm_compare;                                                                           \
                    struct scc_arena btm_arena;                                                                         \
                    unsigned char const btm_kvoff;                                                                      \
                    unsigned char btm_dynalloc;                                                                         \
                    unsigned char btm_fwoff;                                                                            \
                    unsigned char btm_bkoff;                                                                            \
                } btm0;                                                                                                 \
                scc_btmap_impl_pair(keytype, valuetype) btm_curr;                                                       \
            } btm1;                                                                                                     \
            scc_btmnode_impl_layout(keytype, valuetype,order) btm_rootmem[];                                            \
        }                                                                                                               \
    )

//! .. _scc_btmap_with_order:
//! .. c:function:: void *scc_btmap_with_order(keytype, valuetype, scc_bcompare compare, unsigned order)
//!
//!     Instantiate a B-tree map of :ref:`order <btree_order>` :code:`order`, storing instances
//!     of the given key and value types, the former being compared using the supplied :code:`compare` function.
//!
//!     The map is constructed in the frame of the calling function. For more information, see
//!     :ref:`Scope and Lifetimes <scope_and_lifetimes>`.
//!
//!     .. seealso:
//!
//!         :ref:`scc_btmap_with_order_dyn <scc_btmap_with_order_dyn>` for constructing a ``btmap``
//!         with the given order on the heap.
//!
//!     :param keytype: The type of the keys to be stored in the ``btmap``
//!     :param valuetype: The type of the values to be stored in the ``btmap``
//!     :param compare: Pointer to the comparison function to use
//!     :param order: The :ref:`order <btree_order>` of the B-tree map. Must be a value
//!                   greater than 2
//!     :returns: An opaque pointer to a ``btmap`` allocated in the frame of the calling function,
//!               or ``NULL`` if the order is invalid
#define scc_btmap_with_order(keytype, valuetype, compare, order)                                                        \
    scc_btmap_impl_with_order(&(scc_btmap_impl_layout(keytype, valuetype, order)) {                                     \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = order,                                                                                 \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, order),                                   \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, order),                                 \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, order)),                     \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        scc_btmap_impl_curroff(keytype, valuetype, order),                                                              \
        scc_btmap_impl_rootoff(keytype, valuetype, order)                                                               \
    )

//! .. _scc_btmap_new:
//! .. c:function:: void *scc_btmap_new(keytype, valuetype, scc_bcompare compare)
//!
//!     Instantiate a ``btmap`` of the configured :ref:`default order <scc_btmap_default_order>`
//!     storing instances of the given key and value types and using :code:`compare` for comparison.
//!
//!     Calling :code:`scc_btmap_new` is entirely equivalent to calling
//!     :code:`scc_btmap_with_order` with the :code:`order` parameter set to
//!     :code:`SCC_BTMAP_DEFAULT_ORDER`. See :ref:`scc_btmap_with_order <scc_btmap_with_order>`
//!     for more information.
//!
//!     The call cannot fail. The ``btmap`` is constructed on the caller's stack.
//!
//!     .. seealso:
//!
//!         :ref:`scc_btmap_new_dyn <scc_btmap_new_dyn>` for constructing a ``btmap``
//!         with default order on the heap.
//!
//!     :param keytype: The type of the keys to be stored in the map
//!     :param valuetype: The type of the values to be stored in the map
//!     :param compare: Pointer to the comparison function to use
//!     :returns: An opaque pointer to a ``btmap`` allocated in the frame of the calling function
#define scc_btmap_new(keytype, valuetype, compare)                                                                      \
    scc_btmap_impl_new(&(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)) {                          \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = SCC_BTMAP_DEFAULT_ORDER,                                                               \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                 \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),               \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)),   \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        scc_btmap_impl_curroff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                                            \
        scc_btmap_impl_rootoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)                                             \
    )

//! .. _scc_btmap_with_order_dyn:
//! .. c:function:: void *scc_btmap_with_order_dyn(keytype, valuetype, scc_bcompare compare, unsigned order)
//!
//!     Like :ref:`scc_btmap_with_order <scc_btmap_with_order>` except for the ``btmap``
//!     being allocated on the heap rather than on the caller's stack
//!
//!     :param keytype: The type of the keys to be stored in the ``btmap``
//!     :param valuetype: The type of the values to be stored in the ``btmap``
//!     :param compare: Pointer to the comparison function to use
//!     :param order: The :ref:`order <btree_order>` of the B-tree map. Must be a value
//!                   greater than 2
//!     :returns: An opaque pointer to a ``btmap`` allocated in the frame of the calling function,
//!               or ``NULL`` if either the order is invalid or if memory allocation failed
#define scc_btmap_with_order_dyn(keytype, valuetype, compare, order)                                                    \
    scc_btmap_impl_with_order_dyn(&(scc_btmap_impl_layout(keytype, valuetype, order)) {                                 \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = order,                                                                                 \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, order),                                   \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, order),                                 \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, order)),                     \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        sizeof(scc_btmap_impl_layout(keytype, valuetype, order)),                                                       \
        scc_btmap_impl_curroff(keytype, valuetype, order),                                                              \
        scc_btmap_impl_rootoff(keytype, valuetype, order)                                                               \
    )

//! .. _scc_btmap_new_dyn:
//! .. c:function:: void *scc_btmap_new_dyn(keytype, valuetype, scc_bcompare compare)
//!
//!     Like :ref:`scc_btmap_new <scc_btmap_new>` except for the fact that the ``btmap`` is allocated
//!     entirely on the heap.
//!
//!     .. note::
//!
//!         Unlike ``scc_btmap_new``, a call to ``scc_btmap_new_dyn`` may fail. The returned
//!         pointer should always be checked for ``NULL``.
//!
//!     :param keytype: The type of the keys to be stored in the map
//!     :param valuetype: The type of the values to be stored in the map
//!     :param compare: Pointer to the comparison function to use
//!     :returns: An opaque pointer to a ``btmap`` allocated on the heap, or ``NULL`` on allocation failure
#define scc_btmap_new_dyn(keytype, valuetype, compare)                                                                  \
    scc_btmap_impl_new_dyn(&(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)) {                      \
            .btm1 = {                                                                                                   \
                .btm0 = {                                                                                               \
                    .btm_order = SCC_BTMAP_DEFAULT_ORDER,                                                               \
                    .btm_keyoff = scc_btmnode_impl_keyoff(keytype),                                                     \
                    .btm_valoff = scc_btmnode_impl_valoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                 \
                    .btm_linkoff = scc_btmnode_impl_linkoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),               \
                    .btm_keysize = sizeof(keytype),                                                                     \
                    .btm_valsize = sizeof(valuetype),                                                                   \
                    .btm_arena = scc_arena_new(scc_btmnode_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)),   \
                    .btm_compare = compare,                                                                             \
                    .btm_kvoff = scc_btmap_impl_pair_valoff(keytype, valuetype)                                         \
                },                                                                                                      \
            },                                                                                                          \
        },                                                                                                              \
        sizeof(scc_btmap_impl_layout(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)),                                     \
        scc_btmap_impl_curroff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER),                                            \
        scc_btmap_impl_rootoff(keytype, valuetype, SCC_BTMAP_DEFAULT_ORDER)                                             \
    )

//? .. _scc_btmap_impl_new:
//? .. c:function:: void *scc_btmap_impl_new(void *base, size_t coff, size_t rootoff)
//?
//?     Initialize the given B-tree map base struct and return the address of its
//?     :ref:`btm_curr <kvpair_btm_curr>` member
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of the :ref:`struct scc_btmap_base <scc_btmap_base>` structure
//?                  of the map
//?     :param coff: Base-relative offset of the :ref:`btm_curr <kvpair_btm_curr>` member
//?                  in the :code:`base` struct
//?     :param rootoff: Base-relative offset of the memory allocated for the root node
//?     :returns: Address of a handle suitable for referring to the given B-tree map
void *scc_btmap_impl_new(void *base, size_t coff, size_t rootoff);

//? .. _scc_btmap_impl_new_dyn
//? .. c:function:: void *scc_btmap_impl_new_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff)
//?
//?     Allocate a new ``btmap`` on the heap and initialize it. Return the address of its
//?     :ref:`btm_curr <kvpair_btm_curr>` member
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param sbase: Address of a :ref:`struct scc_btmap_base <scc_btmap_base>` containing
//?                   values to be set in the new ``btmap`` base struct.
//?     :param basesz: Size of the ``btmap`` for a particular choice of key and value type
//?     :param coff: Base-relative offset of the :ref:`btm_curr <kvpair_btm_curr>` member
//?                  in the ``base`` struct
//?     :param rootoff: Base-relative offset of the memory allocated for the root node
//?     :returns: Address of a handle suitable for referring to the given ``btmap``, or
//?               ``NULL`` on allocation failure
void *scc_btmap_impl_new_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff);

//? .. c:function:: void *scc_btmap_impl_with_order(void *base, size_t coff, size_t rootoff)
//?
//?     Like :ref:`scc_btmap_impl_new <scc_btmap_impl_new>` but verifies the order
//?     in the base structure. Should it prove invalid, the function returns NULL
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param coff: Base-relative offset of the :ref:`btm_curr <kvpair_btm_curr>` member
//?                  in the base struct
//?     :param rootoff: Base-relative offset of the memory allocated for the root node
//?     :returns: Address of a handle suitable for referring to the given ``btmap``,
//?               or :code:`NULL` if the order field is invalid
inline void *scc_btmap_impl_with_order(void *base, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btmap_base *)base)->btm_order;
    if(order < 3u) {
        return 0;
    }
    return scc_btmap_impl_new(base, coff, rootoff);
}

//? .. c:function:: void *scc_btmap_impl_with_order_dyn(void *sbase, size_t coff, size_t rootoff)
//?
//?     Like :ref:`scc_btmap_impl_new_dyn <scc_btmap_impl_new_dyn>` but verifies the order
//?     in the base structure. Should it prove invalid, the function returns NULL
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param coff: Base-relative offset of the :ref:`btm_curr <kvpair_btm_curr>` member
//?                  in the base struct
//?     :param rootoff: Base-relative offset of the memory allocated for the root node
//?     :returns: Address of a handle suitable for referring to the given ``btmap``,
//?               or :code:`NULL` if the order field is invalid
inline void *scc_btmap_impl_with_order_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btmap_base *)sbase)->btm_order;
    if(order < 3u) {
        return 0;
    }
    return scc_btmap_impl_new_dyn(sbase, basesz, coff, rootoff);
}

//? .. c:function:: size_t scc_btmap_impl_npad(void const *btmap)
//?
//?     Compute the number of padding bytes between :ref:`btm_curr <kvpair_btm_curr>`
//?     and :ref:`btm_fwoff <unsigned_char_btm_fwoff>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btmap: Handle to the B-tree map
//?     :returns: The number of padding bytes between :code:`btm_fwoff` and :code:`btm_curr`
inline size_t scc_btmap_impl_npad(void const *btmap) {
    return ((unsigned char const *)btmap)[-1] + sizeof(unsigned char);
}

//? .. c:macro:: scc_btmap_impl_base_qual(btmap, qual)
//?
//?     Obtain qualified pointer to the :ref:`struct scc_btmap_base <scc_btmap_base>`
//?     corresponding to the given B-tree map handle
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btmap: B-tree map handle
//?     :param qual: Optioal qualifiers to apply to the returned pointer
//?     :returns: Suitably qualified pointer to the base address of the given B-tree map
#define scc_btmap_impl_base_qual(btmap, qual)                                                       \
    scc_container_qual(                                                                             \
        (unsigned char qual *)(btmap) - scc_btmap_impl_npad(btmap),                                 \
        struct scc_btmap_base,                                                                      \
        btm_fwoff,                                                                                  \
        qual                                                                                        \
    )

//? .. c:macro:: scc_btmap_impl_base(btmap)
//?
//?     Obtain unqualified pointer to the base address of the given
//?     :code:`btmap`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btmap: B-tree map handle
//?     :returns: Base address of the given B-tree map
#define scc_btmap_impl_base(btmap)                                                                  \
    scc_btmap_impl_base_qual(btmap,)

//! .. c:function:: void scc_btmap_free(void *btmap)
//!
//!     Reclaim memory allocated for :c:expr:`btmap`. The parameter must
//!     refer to a valid B-tree map returned by :ref:`scc_btmap_new <scc_btmap_new>` or
//!     :ref:`scc_btmap_with_order <scc_btmap_with_order>`.
//!
//!     :param btmap: B-tree map handle
void scc_btmap_free(void *btmap);

//! .. c:function:: size_t scc_btmap_order(void const *btmap)
//!
//!     Return the order of the given B-tree map
//!
//!     :param btmap: B-tree map handle
//!     :returns: The order of the given B-tree map
inline size_t scc_btmap_order(void const *btmap) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    return base->btm_order;
}

//! .. c:function:: size_t scc_btmap_size(void const *btmap)
//!
//!     Query the size of the given B-tree map
//!
//!     :param btmap: B-tree map handle
//!     :returns: Size of the given B-tree map
inline size_t scc_btmap_size(void const *btmap) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    return base->btm_size;
}

//? .. c:function:: _Bool scc_btmap_impl_insert(void *btmapaddr)
//?
//?     Internal insertion function. Attempt to insert the value stored at
//?     :c:texpr:`*(void **)btmapaddr` in the map.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btmapaddr: Address of the B-tree map handle
//?     :returns: A :code:`_Bool` indicating whether insertion took place
//?     :retval true: Insertion successful
//?     :retval false: The value was already in the map, or allocation failure
_Bool scc_btmap_impl_insert(void *btmapaddr);

//! .. c:function:: _Bool scc_btmap_insert(void *btmapaddr, keytype key, valuetype value)
//!
//!     Insert the given value into the B-tree map. If the key already exists in the
//!     map, its associated value is overwritten.
//!
//!     The :code:`value` parameter must not necessarily be the same type as the one
//!     with which the B-tree map was instantiated. If it is not, it is implicitly converted
//!     to the type stored in the map.
//!
//!     :param btmapaddr: Address of the B-tree map handle
//!     :param key: The key to associate the value to be inserted with
//!     :param value: The value to insert in the map
//!     :returns: A :code:`_Bool` indicating whether the insertion took place
//!     :retval true: The key-value pair was inserted
//!     :retval false: Memory allocation failure
//!
//!     .. code-block:: C
//!         :caption: Insert pairs (1, 2), (2, 3), (1, 8) in a B-tree map
//!
//!         extern int compare(void const *l, void const *r);
//!
//!         scc_btmap(int, int) btmap = scc_btmap_new(int, int, compare);
//!
//!         /* Insert first pair */
//!         if(!scc_btmap_insert(&btmap, 1, 2)) {
//!             /* Error */
//!         }
//!         /* Map contains (1, 2) */
//!
//!         /* Insert with new key not in map */
//!         if(!scc_btmap_insert(&btmap, 2, 3)) {
//!             /* Error */
//!         }
//!         /* Map now contains (1, 2), (2, 3) */
//!
//!         /* Insert with key already in map*/
//!         if(!scc_btmap_insert(&btmap, 1, 8)) {
//!             /* Error */
//!         }
//!         /* Map now contains (1, 8), (2, 3) */
//!
//!         scc_btmap_free(btmap);
#define scc_btmap_insert(btmapaddr, key, value)                                                         \
    scc_btmap_impl_insert((                                                                             \
            (*(btmapaddr))->btm_key = (key),                                                            \
            (*(btmapaddr))->btm_value = (value),                                                        \
            (btmapaddr)                                                                                 \
        )                                                                                               \
    )

//? .. c:function:: void *scc_btmap_impl_find(void *btmap)
//?
//?     Internal search function. Attempts to located the key stored at :code:`btmap`
//?     in the ``btmap`` and, if found, return it's corresponding value
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btmap: Handle to the btmap
//?     :returns: An unqualified pointer to the value corresponding to the
//?               key stored at :code:`btmap`, or :code:`NULL` if the key
//?               is not found in the map
void *scc_btmap_impl_find(void *btmap);

//! .. c:function:: void *scc_btmap_find(void *btmap, keytype key)
//!
//!     Search for and, if found, return a pointer to the value corresponding to
//!     the given key.
//!
//!     The :code:`key` parameter must not necessarily be the same type as the one
//!     with which the :code:`btmap` was instantiated. If it is not, it is
//!     implicitly converted to the key type stored in the map.
//!
//!     :param btmap: Handle to the btmap
//!     :param key: The key to search for
//!     :returns: An unqualified pointer to the value corresponding to the given
//!               :code:`key` argument, or :code:`NULL` if the key was not found
//!               in the map
#define scc_btmap_find(btmap, key)                                                                      \
    scc_btmap_impl_find((                                                                               \
            ((btmap)->btm_key = (key)), (btmap)                                                         \
        )                                                                                               \
    )

//? .. c:function:: _Bool scc_btmap_impl_remove(void *btmap)
//?
//?     Internal removal function. Attempts to find and remove the key-value pair
//?     identified by the key stored in the :code:`btm_curr` field.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btmap: Handle to the ``btmap``
//?     :returns: :code:`true` if the key in :code:`btm_curr` field was found
//?               and successfully removed. :code:`false` if no such key
//?               is found in the map
_Bool scc_btmap_impl_remove(void *btmap);

//! .. c:function:: _Bool scc_btmap_remove(void btmap, ketype key)
//!
//!     Find and remove the key-value pair identified by the supplied key.
//!
//!     The :code:`key` parameter must not necessarily be the same type as the
//!     one with which the ``btmap`` was instantiated. If it is not, the key
//!     is implicitly converted to the type stored in the map.
//!
//!     :param btmap: B-tree map handle
//!     :param key: The key identifying the key-value pair to remove
//!     :returns: :code:`true` if the value was removed, :code:`false` if the
//!               key was not found in the map
#define scc_btmap_remove(btmap, key)                                                                    \
    scc_btmap_impl_remove((                                                                             \
            ((btmap)->btm_key = (key)), (btmap)                                                         \
        )                                                                                               \
    )

//! .. c:function:: void *scc_btmap_clone(void const *btmap)
//!
//!     Clone the given ``btmap``, yielding a new copy with the same size and elements.
//!     The new copy is allocated on the heap
//!
//!     :param btmap: The ``btmap`` instance to clone
//!     :returns: A new ``btmap`` instance containing the same key-value pairs as the
//!               supplied parameter, or ``NULL`` on failure
//!
//!     .. code-block:: C
//!         :caption: Clone a ``btmap``
//!
//!         extern int compare(void const *l, void const *r);
//!
//!         scc_btmap(int, int) btmap = scc_btmap_new(int, int, compare);
//!
//!         for(int i = 0; i < 12; ++i) {
//!             assert(scc_btmap_insert(&btmap, i, 2 * i));
//!         }
//!
//!         /* Create an exact copy of the btmap */
//!         scc_btmap(int, int) copy = scc_btmap_clone(btmap);
//!
//!         assert(scc_btmap_size(btmap) == scc_btmap_size(copy));
//!
//!         /* Copy contains the same key-value pairs */
//!         int *old, *new;
//!         for(int i = 0; i < (int)scc_btmap_size(btmap); ++i) {
//!             old = scc_btmap_find(btmap, i);
//!             new = scc_btgmap_find(copy, i);
//!             assert(old && new);
//!             assert(*old == *new);
//!         }
//!
//!         scc_btmap_free(btmap);
//!         /* The copy must be freed too */
//!         scc_btmap_free(copy);
void *scc_btmap_clone(void const *btmap);

#endif /* SCC_BTMAP_H */
