#ifndef SCC_BTREE_H
#define SCC_BTREE_H

#include "scc_arena.h"
#include "scc_assert.h"
#include "scc_bits.h"
#include "scc_mem.h"

#include <stddef.h>

//! .. c:macro:: scc_btree(type)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to a B-tree containing the specified type
//!
//!     :param type: The type to store in the B-tree
#define scc_btree(type) type *

//! .. c:type:: int(*scc_bcompare)(void const *left, void const *right)
//!
//!     Signature of the function used for comparison. The return
//!     value should be as follows
//!
//!     .. list-table:: Flag Bits
//!         :header-rows: 1
//!
//!         * - Value
//!           - Meaning
//!         * - :code:`< 0`
//!           - :code:`left < right`
//!         * - :code:`== 0`
//!           - :code:`left == right`
//!         * - :code:`> 0`
//!           - :code:`left > right`
//!
//!     .. code-block:: C
//!         :caption: Simple implementation comparing two ints
//!
//!         int compare_ints(void const *left, void const *right) {
//!             return *(int const *)left - *(int const *)right;
//!         }
typedef int(*scc_bcompare)(void const *, void const *);

#ifndef SCC_BTREE_DEFAULT_ORDER
//! .. _scc_btree_default_order:
//! .. c:enumerator:: SCC_BTREE_DEFAULT_ORDER
//!
//!     Default :ref:`order <btree_order>` of B-trees
//!     instantiated using :ref:`scc_btree_new <scc_btree_new>`.
//!
//!     The value may be overridden by defining it before including
//!     the header. B-trees of a specific order may be instantiated
//!     using :ref:`scc_btree_with_order <scc_btree_with_order>`
//!
//!     Must be an even number greater than 2. The implementation also
//!     requires that it is smaller than :c:texpr:`SIZE_MAX / 2`, but
//!     chosing values even remotely that size is ill-adviced.
enum { SCC_BTREE_DEFAULT_ORDER = 6 };
#endif /* SCC_BTREE_DEFAULT_ORDER */

scc_static_assert(SCC_BTREE_DEFAULT_ORDER > 2);
scc_static_assert(scc_bits_is_even(SCC_BTREE_DEFAULT_ORDER));

//? .. _scc_btnode_base:
//? .. c:struct:: scc_btnode_base
//?
//?     Generic B-tree node structure
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _unsigned_char_bt_flags:
//?     .. c:var:: unsigned char bt_flags
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
//?     .. _unsigned_short_bt_nkeys:
//?     .. c:var:: unsigned short bt_nkeys
//?
//?         Number of keys stored in the node
//?
//?     .. c:var:: unsigned char bt_data[]
//?
//?         FAM hiding type-specific details. See
//?         :ref:`scc_btnode_impl_layout <scc_btnode_impl_layout>`
//?         for details.
struct scc_btnode_base {
    unsigned char bt_flags;
    unsigned short bt_nkeys;
    unsigned char bt_data[];
};

//? .. _scc_btree_base:
//? .. c:struct:: scc_btree_base
//?
//?     Base structure of the B-tree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _unsigned_short_const_bt_order:
//?     .. c:var:: unsigned short const bt_order
//?
//?         Order of the btree
//?
//?     .. _unsigned_short_const_bt_dataoff:
//?     .. c:var:: unsigned short const bt_dataoff
//?
//?         Offset of each node's data relative its base address
//?
//?     .. _unsigned_short_const_bt_linkoff:
//?     .. c:var:: unsigned short const bt_linkoff
//?
//?         Offset of echo node's array of link pointers
//?
//?     .. _size_t_bt_size:
//?     .. c:var:: size_t bt_size
//?
//?         Size of the btree
//?
//?     .. _struct_scc_btnode_base_bt_root:
//?     .. c:var:: struct scc_btnode_base *bt_root
//?
//?         Address of the root node
//?
//?     .. _struct_scc_arena_bt_arena:
//?     .. c:var:: struct scc_arena bt_arena
//?
//?         Node allocator
//?
//?     .. _scc_bcompare_bt_compare:
//?     .. c:var:: scc_bcompare bt_compare
//?
//?         Pointer to comparison function
//?
//?     .. _unsigned_char_bt_fwoff:
//?     .. c:var:: unsigned char bt_fwoff
//?
//?         Number of padding bytes between the field itself and
//?         :ref:`bt_curr <type_bt_curr>`
//?
//?     .. c:var:: unsigned char bt_data[]
//?
//?         FAM used for storing :ref:`bt_curr <type_bt_curr>`
struct scc_btree_base {
    unsigned short const bt_order;
    unsigned short const bt_dataoff;
    unsigned short const bt_linkoff;
    size_t bt_size;
    struct scc_btnode_base *bt_root;
    struct scc_arena bt_arena;
    scc_bcompare bt_compare;
    unsigned char bt_fwoff;
    unsigned char bt_data[];
};

//? .. _scc_btnode_impl_layout:
//? .. c:macro:: scc_btnode_impl_layout(type, order)
//?
//?     Actual layout of the nodes in a B-tree of order
//?     :code:`order` storing instances of :code:`type`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type of the elements to be stored in the node
//?     :param order: The order to the B-tree
//?
//?     .. c:var:: unsigned char bt_flags
//?
//?         See :ref:`bt_flags <unsigned_char_bt_flags>`.
//?
//?     .. c:var:: unsigned short bt_nkeys
//?
//?         See :ref:`bt_nkeys <unsigned_short_bt_nkeys>`.
//?
//?     .. _type_bt_data:
//?     .. c:var:: type bt_data[order - 1u]
//?
//?         Array storing the data contained in the node. At any
//?         given time, the :code:`bt_nkeys` first nodes of the
//?         array are in use
//?
//?     .. _struct_scc_btnode_base_bt_links:
//?     .. c:var:: struct scc_btnode_base *bt_links[order]
//?
//?         Links to other nodes in the B-tree. At most
//?         the :c:expr:`bt_nkeys + 1u` first nodes are used.
//?
//?         If zero-indexed bit 0 in :code:`bt_flags` is set,
//?         :code:`bt_links[0]` refers to the in-order predecessor
//?         of the node rather than one of its children. Similarly,
//?         zero-indexed bit 1 in :code:`bt_flags` indicates that
//?         :code:`bt_links[bt_nkeys - 1u]` refers to the in-order
//?         successor of the node.
#define scc_btnode_impl_layout(type, order)                                                     \
    struct {                                                                                    \
        unsigned char bt_flags;                                                                 \
        unsigned short bt_nkeys;                                                                \
        type bt_data[(order) - 1u];                                                             \
        struct scc_btnode_base *bt_links[order];                                                \
    }

//? .. c:macro:: scc_btree_impl_layout(type)
//?
//?     Actual layout of a B-tree storing instances of the given
//?     :code:`type`.
//?
//?     The :code:`bt_order` through :code:`bt_fwoff` members are identical
//?     to the ones in :ref:`struct scc_btree_base <scc_btree_base>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:var:: unsigned short const bt_order
//?
//?         See :ref:`bt_order <unsigned_short_const_bt_order>`
//?
//?     .. c:var:: unsigned short const bt_dataoff
//?
//?         See :ref:`bt_dataoff <unsigned_short_const_bt_dataoff>`
//?
//?     .. c:var:: unsigned short const bt_linkoff
//?
//?         See :ref:`bt_linkoff <unsigned_short_const_bt_linkoff>`
//?
//?     .. c:var:: size_t bt_size
//?
//?         See :ref:`bt_size <size_t_bt_size>`
//?
//?     .. c:var:: struct scc_btnode_base *bt_root
//?
//?         See :ref:`bt_root <struct_scc_btnode_base_bt_root>`
//?
//?     .. c:var:: struct scc_arena bt_arena
//?
//?         See :ref:`bt_arena <struct_scc_arena_bt_arena>`
//?
//?     .. c:var:: scc_bcompare bt_compare
//?
//?         See :ref:`bt_compare <scc_bcompare_bt_compare>`
//?
//?     .. c:var:: unsigned char bt_fwoff
//?
//?         See :ref:`bt_fwoff <unsigned_char_bt_fwoff>`
//?
//?     .. _unsigned_char_bt_bkoff:
//?     .. c:var:: unsigned char bt_bkoff
//?
//?         Number of padding bytes between :ref:`bt_fwoff <unsigned_char_bt_fwoff>`
//?         and :ref:`bt_curr <type_bt_curr>`
//?
//?     .. _type_bt_curr:
//?     .. c:var:: type bt_curr[2]
//?
//?         Used for temporary storage for allowing operations on rvalues. Any
//?         elements to be inserted, removed or searched for are first written
//?         here
//?
//?     .. c:var:: struct scc_btnode bt_rootmem
//?
//?         Automatically allocated memory for the root node to ensure that handle
//?         returned by initialization functions such as
//?         :ref:`scc_btree_new <scc_btree_new>` do not have to be :code:`NULL`
//?         checked.
#define scc_btree_impl_layout(type, order)                                                          \
    struct {                                                                                        \
        unsigned short const bt_order;                                                              \
        unsigned short const bt_dataoff;                                                            \
        unsigned short const bt_linkoff;                                                            \
        size_t bt_size;                                                                             \
        struct scc_btnode *bt_root;                                                                 \
        struct scc_arena bt_arena;                                                                  \
        scc_bcompare bt_compare;                                                                    \
        unsigned char bt_fwoff;                                                                     \
        unsigned char bt_bkoff;                                                                     \
        type bt_curr[2];                                                                            \
        scc_btnode_impl_layout(type, order) bt_rootmem;                                             \
    }

//? .. _scc_btree_impl_new:
//? .. c:function:: void *scc_btree_impl_new(void *base, size_t coff, size_t rootoff)
//?
//?     Initialize the given B-tree base struct and return the address of its
//?     :ref:`bt_curr <type_bt_curr>` member
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of the :ref:`struct scc_btree_base <scc_btree_base>` structure
//?                  of the tree
//?     :param coff: Base-relative offset of the :ref:`bt_curr <type_bt_curr>` member
//?                  in the :code:`base` struct
//?     :param rootoff: Base-relative offset of the memory allocated for the root node
//?     :returns: Address of a handle suitable for referring to the given B-tree
void *scc_btree_impl_new(void *base, size_t coff, size_t rootoff);

//? .. c:function:: void *scc_btree_impl_with_order(void *base, size_t coff, size_t rootoff)
//?
//?     Like :ref:`scc_btree_impl_new <scc_btree_impl_new>` but verifies the order
//?     in the base structure. Should it prove invalid, the function returns NULL
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-tree
//?     :param coff: Base-relative offset of the :ref:`bt_curr <type_bt_curr>` member
//?                  in the base struct
//?     :param rootoff: Base-relative offset of the memory allocated for the root node
//?     :returns: Address of a handle suitable for referring to the given B-tree,
//?               or :code:`NULL` if the order field is invalid
inline void *scc_btree_impl_with_order(void *base, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btree_base *)base)->bt_order;
    if(order < 3u || !scc_bits_is_even(order)) {
        return 0;
    }
    return scc_btree_impl_new(base, coff, rootoff);
}

//! .. _scc_btree_with_order:
//! .. c:function:: void *scc_btree_with_order(type, scc_bcompare compare, unsigned order)
//!
//!     Instantiate a B-tree of :ref:`order <btree_order>` :code:`order`, storing instances
//!     of type :code:`type`, each compared using the supplied :code:`compare` function.
//!
//!     The tree is constructed in the frame of the calling function. For more information, see
//!     :ref:`Scope and Lifetimes <scope_and_lifetimes>`.
//!
//!     :param type: The type to be stored in the B-tree
//!     :param compare: Pointer to the comparison function to use
//!     :param order: The :ref:`order <btree_order>` of the B-tree. Must be an even value
//!                   greater than 2
//!     :returns: An opaque pointer to a B-tree allocated in the frame of the calling function,
//!               or NULL if the order is invalid
#define scc_btree_with_order(type, compare, order)                                                  \
    scc_btree_impl_with_order(&(scc_btree_impl_layout(type, order)) {                               \
            .bt_order = order,                                                                      \
            .bt_dataoff = offsetof(scc_btnode_impl_layout(type, order), bt_data),                   \
            .bt_linkoff = offsetof(scc_btnode_impl_layout(type, order), bt_links),                  \
            .bt_arena = scc_arena_new(scc_btnode_impl_layout(type, order)),                         \
            .bt_compare = compare                                                                   \
        },                                                                                          \
        offsetof(scc_btree_impl_layout(type, order), bt_curr),                                      \
        offsetof(scc_btree_impl_layout(type, order), bt_rootmem)                                    \
    )

//! .. _scc_btree_new:
//! .. c:function:: void *scc_btree_new(type, scc_bcompare compare)
//!
//!     Instantiate a B-tree of the configured :ref:`default order <scc_btree_default_order>`
//!     storing instances of the given :code:`type` and using :code:`compare` for comparison.
//!
//!     Calling :code:`scc_btree_new` is entirely equivalent to calling
//!     :code:`scc_btree_with_order` with the :code:`order` parameter set to
//!     :code:`SCC_BTREE_DEFAULT_ORDER`. See :ref:`scc_btree_with_order <scc_btree_with_order>`
//!     for more information.
//!
//!     The call cannot fail.
//!
//!     :param type: The type to be stored in the B-tree
//!     :param compare: Pointer to the comparison function to use
//!     :returns: An opaque pointer to a B-tree allocated in the frame of the calling function
#define scc_btree_new(type, compare)                                                                \
    scc_btree_impl_new(&(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)) {                    \
            .bt_order = SCC_BTREE_DEFAULT_ORDER,                                                    \
            .bt_dataoff = offsetof(scc_btnode_impl_layout(type, SCC_BTREE_DEFAULT_ORDER), bt_data), \
            .bt_linkoff = offsetof(scc_btnode_impl_layout(type, SCC_BTREE_DEFAULT_ORDER), bt_links),\
            .bt_arena = scc_arena_new(scc_btnode_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)),       \
            .bt_compare = compare                                                                   \
        },                                                                                          \
        offsetof(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER), bt_curr),                    \
        offsetof(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER), bt_rootmem)                  \
    )

//? .. c:function:: size_t scc_btree_impl_npad(void const *btree)
//?
//?     Compute the number of padding bytes between :ref:`bt_curr <type_bt_curr>`
//?     and :ref:`bt_fwoff <unsigned_char_bt_fwoff>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btree: Handle to the B-tree
//?     :returns: The number of padding bytes between :code:`bt_fwoff` and :code:`bt_curr`
inline size_t scc_btree_impl_npad(void const *btree) {
    return ((unsigned char const *)btree)[-1] + sizeof(unsigned char);
}

//? .. c:macro:: scc_btree_impl_base_qual(btree, qual)
//?
//?     Obtain qualified pointer to the :ref:`struct scc_btree_base <scc_btree_base>`
//?     corresponding to the given B-tree handle
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btree: B-tree handle
//?     :param qual: Optioal qualifiers to apply to the returned pointer
//?     :returns: Suitably qualified pointer to the base address of the given B-tree
#define scc_btree_impl_base_qual(btree, qual)                                                       \
    scc_container_qual(                                                                             \
        (unsigned char qual *)(btree) - scc_btree_impl_npad(btree),                                 \
        struct scc_btree_base,                                                                      \
        bt_fwoff,                                                                                   \
        qual                                                                                        \
    )

//? .. c:macro:: scc_btree_impl_base(btree)
//?
//?     Obtain unqualified pointer to the base address of the given
//?     :code:`handle`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btree: B-tree handle
//?     :returns: Base address of the given B-tree
#define scc_btree_impl_base(btree)                                                                  \
    scc_btree_impl_base_qual(btree,)

//! .. c:function:: void scc_btree_free(void *btree)
//!
//!     Reclaim memory allocated for :c:expr:`btree`. The parameter must
//!     refer to a valid B-tree returned by :ref:`scc_btree_new <scc_btree_new>` or
//!     :ref:`scc_btree_with_order <scc_btree_with_order>`.
//!
//!     :param btree: B-tree handle
void scc_btree_free(void *btree);

//! .. c:function:: size_t scc_btree_order(void const *btree)
//!
//!     Return the order of the given B-tree
//!
//!     :param btree: B-tree handle
//!     :returns: The order of the given B-tree
inline size_t scc_btree_order(void const *btree) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    return base->bt_order;
}

//! .. c:function:: size_t scc_btree_size(void const *btree)
//!
//!     Query the size of the given B-tree
//!
//!     :param btree: B-tree handle
//!     :returns: Size of the given B-tree
inline size_t scc_btree_size(void const *btree) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    return base->bt_size;
}

//? .. c:function:: _Bool scc_btree_impl_insert(void *btreeaddr, size_t elemsize)
//?
//?     Internal insertion function. Attempt to insert the value stored at
//?     :c:texpr:`*(void **)btreeaddr` in the tree.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btreeaddr: Address of the B-tree handle
//?     :param elemsize: Size of the elements stored in the B-tree
//?     :returns: A :code:`_Bool` indicating whether insertion took place
//?     :retval true: Insertion successful
//?     :retval false: The value was already in the tree, or allocation failure
_Bool scc_btree_impl_insert(void *btreeaddr, size_t elemsize);

//! .. c:function:: _Bool scc_btree_insert(void *btreeaddr, type value)
//!
//!     Attempt to insert the given value into the B-tree.
//!
//!     The :code:`value` parameter must not necessarily be the same type for
//!     which the B-tree was instantiated. If it is not, it is implicitly converted
//!     to the type stored in the tree.
//!
//!     :param btreeaddr: Address of the B-tree handle
//!     :param value: The value to insert in the tree
//!     :returns: A :code:`_Bool` indicating whether the insertion took place
//!     :retval true: The value was inserted
//!     :retval false: The values was already in the tree, or memory allocation failure
#define scc_btree_insert(btreeaddr, value)                                                              \
    scc_btree_impl_insert((**(btreeaddr) = (value), btreeaddr), sizeof(**(btreeaddr)))

#endif /* SCC_BTREE_H */
