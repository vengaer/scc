#ifndef SCC_RBTREE_H
#define SCC_RBTREE_H

#include "arena.h"
#include "mem.h"
#include "pp_token.h"

#include <stddef.h>

//! .. c:macro:: scc_rbtree(type)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to a red-black tree containing the specified type
//!
//!     :param type: The type to store in the red-black tree
#define scc_rbtree(type) type *

//! .. _scc_rbcompare:
//! .. c:type:: int(*scc_rbcompare)(void const *left, void const *right)
//!
//!     Signature of the function used for comparison. The return
//!     value should be as follows
//!
//!     .. list-table:: Comparison return values
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
typedef int(*scc_rbcompare)(void const *, void const *);

//? .. c:enum:: scc_rbcolor
//?
//?     Denotes the color of a node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:enumerator:: scc_rbcolor_black
//?
//?         Used for black nodes
//?
//?     .. c:enumerator:: scc_rbcolor_red
//?
//?         Used for red nodes
enum scc_rbcolor {
    scc_rbcolor_black,
    scc_rbcolor_red
};

//? .. c:enum:: scc_rbdir
//?
//?     Denotes the direction of link traversal
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:enumerator:: scc_rbdir_left
//?
//?         Traverse left link
//?
//?     .. c:enumerator:: scc_rbdir_right
//?
//?         Traverse right link
enum scc_rbdir {
    scc_rbdir_left,
    scc_rbdir_right
};

//? .. c:macro:: scc_rbnode_link_qual(node, idx, qual)
//?
//?     Obtain suitably qualified pointer to the idxth
//?     link of the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node whose link address is to be
//?                  computed
//?     :param idx:  The index of the link
//?     :param qual: Qualifiers to apply to the pointer
//?     :returns:    Address of the idxth link of the given node
#define scc_rbnode_link_qual(node, idx, qual)                       \
    (*(struct scc_rbnode_base *qual *)                              \
        ((unsigned char qual *)&(node)->rn_left +                   \
            (scc_rbnode_link_offset(node)) * (idx)))

//? .. c:macro:: scc_rbnode_link(node, idx)
//?
//?     Obtain unqualified pointer to the idxth
//?     link of the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node whose link address is to be
//?                  computed
//?     :param idx:  The index of the link
//?     :returns:    Address of the idxth link of the given node
#define scc_rbnode_link(node, idx)                                  \
    scc_rbnode_link_qual(node, idx,)

//? .. c:macro:: scc_rbnode_value_qual(base, node, qual)
//?
//?     Compute and return suitably qualified pointer to the
//?     :ref:`rn_value <type_rn_value>` field in the given
//?     node
//?
//?     :param base: Base address of the rbtree
//?     :param node: Base address of the node
//?     :param qual: Qualifiers to apply to the computed pointer
//?     :returns: Address of the :ref:`rn_value <type_rn_value>` field in :code:`node`.
#define scc_rbnode_value_qual(base, node, qual)                     \
    ((void qual *)(((unsigned char qual *)node) +                   \
        ((base)->rb_dataoff)))

//? .. c:macro:: scc_rbnode_value(base, node)
//?
//?     Compute and return unqualified pointer to the
//?     :ref:`rn_value <type_rn_value>` field in the given
//?     node
//?
//?     :param base: Base address of the rbtree
//?     :param node: Base address of the node
//?     :returns: Address of the :ref:`rn_value <type_rn_value>` field in :code:`node`.
#define scc_rbnode_value(base, node)                                \
    scc_rbnode_value_qual(base, node,)

//? .. _scc_rbnode_base:
//? .. c:struct:: scc_rbnode_base
//?
//?     Generic rbtree node base structure
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _struct_scc_rbnode_base_rn_left:
//?     .. c:var:: struct scc_rbnode_base *rn_left
//?
//?         Link to left child
//?
//?     .. _struct_scc_rbnode_base_rn_right:
//?     .. c:var:: struct scc_rbnode_base *rn_right
//?
//?         Link to right child
//?
//?     .. _enum_scc_rbcolor_rn_color:
//?     .. c:var:: enum scc_rbcolor rn_color
//?
//?         Color for the node
//?
//?     .. _unsigned_char_rn_flags:
//?     .. c:var:: unsigned char rn_flags
//?
//?         Internal bitflags
//?
//?     .. c:var:: unsigned char rn_data[]
//?
//?         FAM hiding type-specific details. See :ref:`rn_value <type_rn_value>`
//?         for more information
struct scc_rbnode_base {
    struct scc_rbnode_base *rn_left;
    struct scc_rbnode_base *rn_right;
    enum scc_rbcolor rn_color;
    unsigned char rn_flags;
    unsigned char rn_data[];
};

//? .. c:struct scc_rbsentinel
//?
//?     Structure of the :ref:`sentinel node <struct_scc_rbsentinel_rb_sentinel>`
//?     node in the :ref:`tree base <scc_rbtree_base>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:var:: struct scc_rbnode_base *rs_left
//?
//?         Link to left child
//?
//?     .. c:var:: struct scc_rbnode_base *rs_right
//?
//?         Link to right child. Used only for layout compatibility with
//?         :ref:`scc_rbnode_base <scc_rbnode_base>`.
//?
//?     .. c:var:: enum scc_rbcolor rs_color
//?
//?         Dummy color field to enforce layout compatibility with
//?         :ref:`scc_rbnode_base <scc_rbnode_base>`.
//?
//?     .. c:var:: unsigned char rs_flags
//?
//?         Internal bitflags
struct scc_rbsentinel {
    struct scc_rbnode_base *rs_left;
    struct scc_rbnode_base *rs_right;
    enum scc_rbcolor rs_color;
    unsigned char rs_flags;
};

//? .. _scc_rbtree_base:
//? .. c:struct:: scc_rbtree_base
//?
//?     Base structure of the rbtree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _unsigned_short_rb_dataoff:
//?     .. c:var:: unsigned short rb_dataoff
//?
//?         Offset of each node's data relative its base address
//?
//?     .. _size_t_rb_size:
//?     .. c:var:: size_t rb_size
//?
//?         Size of the rbtree
//?
//?     .. _scc_rbcompare_rb_compare:
//?     .. c:var:: scc_rbcompare rb_compare
//?
//?         Address of comparison function
//?
//?     .. _struct_scc_arena_rb_arena:
//?     .. c:var:: struct scc_arena rb_arena
//?
//?         Node allocator
//?
//?     .. _struct_scc_rbsentinel_rb_sentinel:
//?     .. c:var:: struct scc_rbsentinel rb_sentinel
//?
//?         Sentinel node. rb_sentinel.rs_left holds the
//?         address refers to the tree root
//?
//?     .. _unsigned_char_rb_dynalloc
//?     .. c:var:: unsigned char rb_dynalloc
//?
//?         1 if the base is dynamically allocated, otherwise 0
//?
//?     .. _unsigned_char_rb_fwoff:
//?     .. c:var:: unsigned char rb_fwoff
//?
//?         Number of padding bytes between the field itself and
//?         :ref:`rb_curr <type_rb_curr>`
//?
//?     .. c:var:: unsigned char rb_data[]
//?
//?         FAM hiding type-specific details. See :ref:`scc_rbtree_impl_layout <scc_rbtree_impl_layout>`
//?         for more information.
struct scc_rbtree_base {
    unsigned short rb_dataoff;
    size_t rb_size;
    scc_rbcompare rb_compare;
    struct scc_arena rb_arena;
    struct scc_rbsentinel rb_sentinel;
    unsigned char rb_dynalloc;
    unsigned char rb_fwoff;
    unsigned char rb_data[];
};

//? .. c:macro:: scc_rbnode_impl_layout(type)
//?
//?     Actual layout of the nodes in an rbtree
//?     storing instances of :code:`type`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type of the elements to be stored in the node
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
//?         :ref:`rn_value <type_rn_value>`.
//?
//?     .. _type_rn_value:
//?     .. c:var:: type rn_value
//?
//?         The value stored in the node
#define scc_rbnode_impl_layout(type)                                                        \
    struct {                                                                                \
        struct {                                                                            \
            struct scc_rbnode_base *rn_left;                                                \
            struct scc_rbnode_base *rn_right;                                               \
            enum scc_rbcolor rn_color;                                                      \
            unsigned char rn_flags;                                                         \
            unsigned char rn_bkoff;                                                         \
        } rbn0;                                                                             \
        type rn_value;                                                                      \
    }

//? .. c:macro:: scc_rbnode_impl_valoff(type)
//?
//?     Compute offset of :ref:`rn_value <type_rn_value>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type of the elements to be stored in the node
#define scc_rbnode_impl_valoff(type)                                                        \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct scc_rbnode_base *rn_left;                                            \
                struct scc_rbnode_base *rn_right;                                           \
                enum scc_rbcolor rn_color;                                                  \
                unsigned char rn_flags;                                                     \
                unsigned char rn_bkoff;                                                     \
            } rbn0;                                                                         \
            type rn_value[];                                                                \
        }                                                                                   \
    )

//? .. _scc_rbtree_impl_layout:
//? .. c:macro:: scc_rbtree_impl_layout(type)
//?
//?     Actual layout of the nodes in an rbtree
//?     storing instances of :code:`type`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type of the elements to be stored in the node
//?
//?     .. c:var:: unsigned short rb_dataoff
//?
//?         See :ref:`rb_dataoff <unsigned_short_rb_dataoff>`.
//?
//?     .. c:var:: size_t rb_size
//?
//?         See :ref:`rb_size <size_t_rb_size>`.
//?
//?     .. c:var:: scc_rbcompare rb_compare
//?
//?         See :ref:`rb_compare <scc_rbcompare_rb_compare>`.
//?
//?     .. c:var:: struct scc_arena rb_arena
//?
//?         See :ref:`rb_arena <struct_scc_arena_rb_arena>`.
//?
//?     .. c:var:: struct scc_rbsentinel rb_sentinel
//?
//?         See :ref:`rb_sentinel <struct_scc_rbsentinel_rb_sentinel>`.
//?
//?     .. c:var:: unsigned char rb_dynalloc
//?
//?         See :ref:`rb_dynalloc <unsigned_char_rb_dynalloc>`.
//?
//?     .. c:var:: unsigned char rb_fwoff
//?
//?         See :ref:`rb_fwoff <unsigned_char_rb_fwoff>`.
//?
//?     .. _unsigned_char_rb_bkoff:
//?     .. c:var:: unsigned char rb_bkoff
//?
//?         Number of padding bytes between :ref:`rb_fwoff <unsigned_char_rb_fwoff>`
//?         and :ref:`rb_curr <type_rb_curr>`
//?
//?     .. _type_rb_curr:
//?     .. c:var:: type rb_curr
//?
//?         Used for temporary storage for allowing operations on rvalues. Any
//?         elements to be inserted, removed or searched for are temporarily
//?         stored here
#define scc_rbtree_impl_layout(type)                                                        \
    struct {                                                                                \
        struct {                                                                            \
            unsigned short rb_dataoff;                                                      \
            size_t rb_size;                                                                 \
            scc_rbcompare rb_compare;                                                       \
            struct scc_arena rb_arena;                                                      \
            struct scc_rbsentinel rb_sentinel;                                              \
            unsigned char rb_dynalloc;                                                      \
            unsigned char rb_fwoff;                                                         \
            unsigned char rb_bkoff;                                                         \
        } rb0;                                                                              \
        type rb_curr;                                                                       \
    }

//? .. c:macro:: scc_rbtree_impl_curroff(type)
//?
//?     Compute offset of :ref:`rb_curr <type_rb_curr>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type of the elements to be stored in the node
#define scc_rbtree_impl_curroff(type)                                                       \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                unsigned short rb_dataoff;                                                  \
                size_t rb_size;                                                             \
                scc_rbcompare rb_compare;                                                   \
                struct scc_arena rb_arena;                                                  \
                struct scc_rbsentinel rb_sentinel;                                          \
                unsigned char rb_dynalloc;                                                  \
                unsigned char rb_fwoff;                                                     \
                unsigned char rb_bkoff;                                                     \
            } rb0;                                                                          \
            type rb_curr[];                                                                 \
        }                                                                                   \
    )

//? .. _scc_rbtree_impl_new:
//? .. c:function:: void *scc_rbtree_impl_new(struct scc_rbtree_base *base, size_t coff)
//?
//?     Initialize the given rbtree base struct and return the address of the
//?     :ref:`rb_curr <type_rb_curr>` field in the tree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base:    Address of the :ref:`struct scc_rbtree_base <scc_rbtree_base>`
//?                     structure of the tree.
//?     :param coff:    Base-relative offset of the :ref:`rb_curr <type_rb_curr>` field in
//?                     the base structure
//?     :returns:       Address of a handle suitable for referring to the given rbtree
void *scc_rbtree_impl_new(struct scc_rbtree_base *base, size_t coff);

//? .. c:function:: void *scc_rbtree_impl_new(struct scc_rbtree_base *base, size_t coff)
//?
//?     Like :ref:``scc_rbtree_impl_new <scc_rbtree_impl_new>`` except for the
//?     tree being allocated on the heap.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param treesz: Size of the tree
//?     :param arena: Arena allocator to use
//?     :param compare: Comparison function
//?     :param coff:    Base-relative offset of the :ref:`rb_curr <type_rb_curr>` field in
//?                     the base structure
//?     :param dataoff: Offset of the ``rn_value`` field in each ``struct scc_rbnode``
//?     :returns:       Address of a handle suitable for referring to the given rbtree, or
//?                     ``NULL`` on allocation failure
void *scc_rbtree_impl_new_dyn(size_t treesz, struct scc_arena *arena, scc_rbcompare compare, size_t coff, size_t dataoff);

//! .. _scc_rbtree_new:
//! .. c:function:: void *scc_rbtree_new(type, scc_rbcompare compare)
//!
//!     Instantiate an rbtree storing instances of the given :code:`type`
//!     and using :code:`compare` for comparison.
//!
//!     The call cannot fail.
//!
//!     .. seealso::
//!
//!         :ref:`scc_rbtree_new_dyn <scc_rbtree_new_dyn>` for a dynamically
//!         allocated tree
//!
//!     :param type: The type to be stored in the rbtree
//!     :param compare: Pointer to the comparison function to use
//!     :returns: An opaque pointer to a rbtree allocated in the frame of the calling function
#define scc_rbtree_new(type, compare)                                                       \
    (type *)scc_rbtree_impl_new(                                                            \
        (void *)&(scc_rbtree_impl_layout(type)) {                                           \
            .rb0 = {                                                                        \
                .rb_dataoff = scc_rbnode_impl_valoff(type),                                 \
                .rb_compare = compare,                                                      \
                .rb_arena = scc_arena_new(scc_rbnode_impl_layout(type)),                    \
            },                                                                              \
        },                                                                                  \
        scc_rbtree_impl_curroff(type)                                                       \
   )

//! .. _scc_rbtree_new_dyn:
//! .. c:function:: void *scc_rbtree_new_dyn(type, scc_rbcompare compare)
//!
//!     Like :ref:`scc_rbtree_new` but the tree is allocated on the heap
//!     rater than the stack.
//!
//!     .. note::
//!
//!         Unlike ``scc_rbtree_new``, ``scc_rbtree_new_dyn`` may fail. The
//!         returned pointer should always be checked against ``NULL``
//!
//!     :param type: The type to be stored in the rbtree
//!     :param compare: Pointer to the comparison function to use
//!     :returns: An opaque pointer to a rbtree allocated in the frame of the calling function
#define scc_rbtree_new_dyn(type, compare)                                                   \
    (type *)scc_rbtree_impl_new_dyn(                                                        \
        sizeof(scc_rbtree_impl_layout(type)),                                               \
        &scc_arena_new(scc_rbnode_impl_layout(type)),                                       \
        compare,                                                                            \
        scc_rbtree_impl_curroff(type),                                                      \
        scc_rbnode_impl_valoff(type)                                                        \
    )

//? .. c:function:: size_t scc_rbtree_impl_npad(void const *rbtree)
//?
//?     Compute the number of padding bytes between :ref:`rb_curr <type_rb_curr>`
//?     and :ref:`rb_fwoff <unsigned_char_rb_fwoff>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: Handle to the rbtree
//?     :returns: The number of padding bytes between :code:`rb_fwoff` and :code:`rb_curr`
inline size_t scc_rbtree_impl_npad(void const *rbtree) {
    return ((unsigned char const *)rbtree)[-1] + sizeof(unsigned char);
}

//? .. c:function:: size_t scc_rbnode_link_offset(struct scc_rbnode_base const *node)
//?
//?     Compute offset between the :ref:`rn_left <struct_scc_rbnode_base_rn_left>`
//?     and :ref:`rn_right <struct_scc_rbnode_base_rn_right>` pointers in an
//?     rbnode
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node to use for the computation
//?     :returns:    The offset between the two links of an rbnode
inline size_t scc_rbnode_link_offset(struct scc_rbnode_base const *node) {
    return ((unsigned char const*)&node->rn_right) - ((unsigned char const *)&node->rn_left);
}

//? .. c:function:: _Bool scc_rbnode_thread(struct scc_rbnode_base const *node, enum scc_rbdir dir)
//?
//?     Determine if the given node's link in the specified direction is a thread
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node whose link is to be checked
//?     :param dir:  The direction of the link to be checked
//?     :returns: :code:`true` if the link in the given direction is a thread, otherwise :code:`false`.
inline _Bool scc_rbnode_thread(struct scc_rbnode_base const *node, enum scc_rbdir dir) {
    return node->rn_flags & (1u << dir);
}

//? .. c:function:: size_t scc_rbnode_bkoff(void const *iter)
//?
//?     Read the :ref:`rn_bkoff <unsigned_char_rn_bkoff>` field of the rbnode
//?     correponding to the given value address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param iter: Address of the :ref:`rn_value <type_rn_value>` field in the
//?                  currently traversed rbnode
//?     :returns:    Value of the :ref:`rn_bkoff <unsigned_char_rn_bkoff>` in the
//?                  node containing to the given value
inline size_t scc_rbnode_bkoff(void const *iter) {
    return ((unsigned char const *)iter)[-1];
}

//? .. c:macro:: scc_rbnode_impl_base_qual(valaddr, qual)
//?
//?     Obtain suitably qualified pointer to the base
//?     address of the node containing the given value address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param valaddr: Address of the :ref:`rn_value <type_rn_value>` field
//?                     in the node
//?     :param qual:    Qualifiers to apply to the pointer
//?     :returns:       Base address of the :ref:`struct scc_rbnode_base <scc_rbnode_base>`
//?                     corresponding to the given value address
#define scc_rbnode_impl_base_qual(valaddr, qual)                    \
    scc_container_qual(                                             \
        (unsigned char qual *)valaddr -                             \
            scc_rbnode_bkoff(valaddr),                              \
        struct scc_rbnode_base,                                     \
        rn_data,                                                    \
        qual                                                        \
    )

//? .. c:macro:: scc_rbnode_impl_base(valaddr)
//?
//?     Obtain unqualified pointer to the base address of the node
//?     containing the given value address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param valaddr: Address of the :ref:`rn_value <type_rn_value>` field
//?                     in the node
//?     :returns:       Base address of the :ref:`struct scc_rbnode_base <scc_rbnode_base>`
//?                     corresponding to the given value address
#define scc_rbnode_impl_base(valaddr)                               \
    scc_rbnode_impl_base_qual(valaddr,)


//? .. c:macro:: scc_rbtree_impl_base_qual(rbtree, qual)
//?
//?     Obtain qualified pointer to the :ref:`struct scc_rbtree_base <scc_rbtree_base>`
//?     corresponding to the given rbtree handle
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: rtree handle
//?     :param qual: Optioal qualifiers to apply to the returned pointer
//?     :returns: Suitably qualified pointer to the base address of the given rbtree
#define scc_rbtree_impl_base_qual(rbtree, qual)                                             \
    scc_container_qual(                                                                     \
        (unsigned char qual *)(rbtree) - scc_rbtree_impl_npad(rbtree),                      \
        struct scc_rbtree_base,                                                             \
        rb_fwoff,                                                                           \
        qual                                                                                \
    )

//? .. c:macro:: scc_rbtree_impl_base(rbtree)
//?
//?     Obtain unqualified pointer to the base address of the given
//?     :code:`rbtree`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: rbtree handle
//?     :returns: Base address of the given rbtree
#define scc_rbtree_impl_base(rbtree)                                                        \
    scc_rbtree_impl_base_qual(rbtree,)

//! .. c:function:: size_t scc_rbtree_size(void const *rbtree)
//!
//!     Query the size of the given rbtree
//!
//!     :param rbtree: rbtree handle
//!     :returns: Size of the given rbtree
inline size_t scc_rbtree_size(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    return base->rb_size;
}

//! .. c:function:: _Bool scc_rbtree_empty(void const *rbtree)
//!
//!     Determine whether the given rbtree is empty
//!
//!     :param rbtree: rbtree handle
//!     :returns: :code:`true` if the tree is empty, otherwise :code:`false`.
inline _Bool scc_rbtree_empty(void const *rbtree) {
    return !scc_rbtree_size(rbtree);
}

//! .. c:function:: void scc_rbtree_clear(void *rbtree)
//!
//!     Remove all elements in the given tree
//!
//!     :param rbtree: The tree to clear
void scc_rbtree_clear(void *rbtree);

//! .. c:function:: void scc_rbtree_free(void *rbtree)
//!
//!     Reclaim memory allocated for :c:expr:`rbtree`. The parameter must
//!     refer to a valid rbtree returned by :ref:`scc_rbtree_new <scc_rbtree_new>`
//!
//!     :param rbtree: rbtree handle
void scc_rbtree_free(void *rbtree);

//? .. c:function:: void *scc_rbtree_impl_generic_insert(void *rbtreeaddr, size_t elemsize)
//?
//?     Internal insertion function. Attempt to insert the value stored at
//?     :c:texpr:`*(void **)rbtreeaddr` in the tree. If the tree already contains
//?     and instance of the value in question, the function returns an address to
//?     said in-tree element. Otherwise, either rbtreeaddr or ``NULL`` are returned,
//?     depending whether or not the internal memory allocation was successful.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtreeaddr: Address of the rbtree handle
//?     :param elemsize: Size of the elements stored in the rbtree
//?     :returns: The address of one of the following
//?     :retval NULL: Memory allocation failure
//?     :retval *(void **)rbtreeaddr: The value was successfully inserted
//?     :retval other address: The tree already contained the value to be inserted,
//?                            the address returned is that of the internal element
void *scc_rbtree_impl_generic_insert(void *rbtreeaddr, size_t elemsize);

//? .. c:function:: _Bool scc_rbtree_impl_insert(void *rbtreeaddr, size_t elemsize)
//?
//?     Internal insertion function. Attempt to insert the value stored at
//?     :c:texpr:`*(void **)rbtreeaddr` in the tree.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtreeaddr: Address of the rbtree handle
//?     :param elemsize: Size of the elements stored in the rbtree
//?     :returns: A :code:`_Bool` indicating whether insertion took place
//?     :retval true: Insertion successful
//?     :retval false: The value was already in the tree, or allocation failure
inline _Bool scc_rbtree_impl_insert(void *rbtreeaddr, size_t elemsize) {
    return scc_rbtree_impl_generic_insert(rbtreeaddr, elemsize) == *(void **)rbtreeaddr;
}

//! .. c:function:: _Bool scc_rbtree_insert(void *rbtreeaddr, type value)
//!
//!     Insert the given value into the rbtree.
//!
//!     The :code:`value` parameter must not necessarily be the same type as the one
//!     with which the rbtree was instantiated. If it is not, it is implicitly converted
//!     to the type stored in the tree.
//!
//!     :param rbtreeaddr: Address of the rbtree handle
//!     :param value: The value to insert in the tree
//!     :returns: A :code:`_Bool` indicating whether the insertion took place
//!     :retval true: The value was inserted
//!     :retval false: The values was already in the tree, or memory allocation failure
//!
//!     .. code-block:: C
//!         :caption: Insert 0, 1, 2, 3 and 4 in an rbtree
//!
//!         extern int compare(void const *l, void const *r);
//!
//!         scc_rbtree(int) rbtree = scc_rbtree_new(int, compare);
//!
//!         for(int i = 0; i < 5; ++i) {
//!             if(!scc_rbtree_insert(&rbtree, i)) {
//!                 fprintf(stderr, "Could not insert %d\n", i);
//!             }
//!         }
//!
//!         /* Use rbtree */
//!
//!         scc_rbtree_free(rbtree);
#define scc_rbtree_insert(rbtreeaddr, value)                                                \
    scc_rbtree_impl_insert((**(rbtreeaddr) = (value), rbtreeaddr), sizeof(**(rbtreeaddr)))

//? .. c:function:: void const *scc_rbtree_impl_find(void *rbtree)
//?
//?     Internal search function. Attempts to find the value stored at
//?     :code:`rbtree` in the tree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: rbtree handle
//?     :returns: A const-qualified pointer to a matching element in the tree, or
//?               :code:`NULL` if no such element is found
void const *scc_rbtree_impl_find(void const *rbtree);

//! .. c:function:: void const *scc_rbtree_find(void const *rbtree, type value)
//!
//!     Search for, and if found, return a pointer to, an element matching the
//!     given value.
//!
//!     The :code:`value` parameter must not necessarily be the same type as the one
//!     with which the rbtree was instantiated. If it is not, it is implicitly
//!     converted to the type stored in the tree.
//!
//!     :param rbtree: Handle to the rbtree
//!     :param value: The value to search for
//!     :returns: A const-qualified pointer to the element in the tree if found, otherwise
//!               :code:`NULL`
#define scc_rbtree_find(rbtree, value)                                                      \
    scc_rbtree_impl_find((*(rbtree) = (value), (rbtree)))

//? .. c:function:: _Bool scc_rbtree_impl_remove(void *rbtree, size_t elemsize)
//?
//?     Internal removal function. Attempts to find and remove the value stored
//?     in the :code:`rb_curr` field.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: rbtree handle
//?     :param elemsize: Size of the the elements in the rbtree
//?     :returns: :code:`true` if the value in :code:`rb_curr` field was found
//?               and successfully removed. :code:`false` if no such value was
//?               found in the tree
_Bool scc_rbtree_impl_remove(void *rbtree, size_t elemsize);

//! .. c:function:: _Bool scc_rbtree_remove(void *rbtree, type value)
//!
//!     Find and remove the specified value in the given rbtree.
//!
//!     The :code:`value` parameter must not necessarily be the same type as the
//!     one with which the rbtree was instantiated. If it is not, the value is
//!     implicitly converted to the type stored in the tree.
//!
//!     :param rbtree: Handle to the rbtree
//!     :param value: Value to remove
//!     :returns: :code:`true` if the value was removed, :code:`false` if no
//!               such value was found
#define scc_rbtree_remove(rbtree, value)                                                    \
    scc_rbtree_impl_remove((*(rbtree) = (value), (rbtree)), sizeof(*(rbtree)))

//? .. c:function:: void const *scc_rbtree_impl_leftmost_value(void const *rbtree)
//?
//?     Find and return the address of the value in the leftmost node in the given tree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: rbtree handle
//?     :returns:      Address of the value in the leftmost node in the tree
void const *scc_rbtree_impl_leftmost_value(void const *rbtree);

//? .. c:function:: void const *scc_rbtree_impl_rightmost_value(void const *rbtree)
//?
//?     Find and return the address of the value in the rightmost node in the given tree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: rbtree handle
//?     :returns:      Address of the value in the rightmost node in the tree
void const *scc_rbtree_impl_rightmost_value(void const *rbtree);

//? .. c:function:: void const *scc_rbtree_impl_successor(void const *iter)
//?
//?     Find and return the address of the in-order successor of the given address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param iter: Address of the :ref:`rn_value <type_rn_value>` field in
//?                  the currently traversed node
//?     :returns:    Address of the in-order successor of the given node
void const *scc_rbtree_impl_successor(void const *iter);

//? .. c:function:: void const *scc_rbtree_impl_predecessor(void const *iter)
//?
//?     Find and return the address of the in-order predecessor of the given address
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param iter: Address of the :ref:`rn_value <type_rn_value>` field in
//?                  the currently traversed node
//?     :returns:    Address of the in-order predecessor of the given node
void const *scc_rbtree_impl_predecessor(void const *iter);

//? .. c:function:: void const *scc_rbtree_impl_iterstop(void const *rbtree)
//?
//?     Compute address to be used as stop marker in :ref:`scc_rbtree_foreach <scc_rbtree_foreach>`
//?     and :ref:`scc_rbtree_foreach_reversed <scc_rbtree_foreach_reversed>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: Tree handle
//?     :returns:      An address within the rbtree struct to be used
//?                    as stop address for the iteration
inline void const *scc_rbtree_impl_iterstop(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    return (unsigned char const *)&base->rb_sentinel + base->rb_dataoff;
}

//? .. c:function:: void *scc_rbtree_impl_clone(void const *rbtree, size_t elemsize)
//?
//?     Internal clone function. See :ref:`scc_rbtree_clone <scc_rbtree_clone>`
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: The ``rbtree`` to clone
//?     :param elemsize: Size of the elements in the tree
//?     :returns: A ``rbtree`` instance containing the same values as the
//?               given :code:`rbtree` parameter, or ``NULL`` on failure
void *scc_rbtree_impl_clone(void const *rbtree, size_t elemsize);

//! .. _scc_rbtree_clone:
//! .. c:function:: void *scc_rbtree_clone(void const *rbtree)
//!
//!     Clone the given ``rbtree``, yielding a new copy with the same size and elements.
//!     The new copy is allocated on the heap
//!
//!     :param rbtree: The ``rbtree`` instance to clone
//!     :returns: A new ``rbtree`` instance containing the same key-value pairs as the
//!               supplied parameter, or ``NULL`` on failure
//!
//!     .. code-block:: C
//!         :caption: Clone an ``rbtree``
//!
//!         extern int compare(void const *l, void const *r);
//!
//!         scc_rbtree(int) rbtree = scc_rbtree_new(int, compare);
//!
//!         for(int i = 0; i < 12; ++i) {
//!             assert(scc_rbtree_insert(&rbtree, i));
//!         }
//!
//!         /* Create an exact copy of the rbtree */
//!         scc_rbtree(int) copy = scc_rbtree_clone(rbtree);
//!
//!         assert(scc_rbtree_size(rbtree) == scc_rbtree_size(copy));
//!
//!         /* Copy contains the same key-value pairs */
//!         int const *old, *new;
//!         for(int i = 0; i < (int)scc_rbtree_size(rbtree); ++i) {
//!             old = scc_rbtree_find(rbtree, i);
//!             new = scc_rbtree_find(copy, i);
//!             assert(old && new);
//!             assert(*old == *new);
//!         }
//!
//!         scc_rbtree_free(rbtree);
//!         /* Free the copy */
//!         scc_rbtree_free(copy);
#define scc_rbtree_clone(rbtree)                                                            \
    scc_rbtree_impl_clone(rbtree, sizeof(*(rbtree)))

//! .. _scc_rbtree_foreach:
//! .. c:macro:: scc_rbtree_foreach(iter, rbtree)
//!
//!     In-order iteration of the nodes in the rbtree. The macro results in a scope
//!     executed once for each node in the tree with :code:`iter` referring to each such
//!     node, in order.
//!
//!     :param iter:   Const-qualified pointer to the type of element stored in the tree
//!     :param rbtree: rbtree handle
//!
//!     .. code-block:: C
//!         :caption: Iterate over and print each element in a tree
//!
//!         extern scc_rbtree(int) rbtree;
//!
//!         // Iterator
//!         int const *iter;
//!
//!         scc_rbtree_foreach(iter, rbtree) {
//!             printf("%d\n", *iter);
//!         }
#define scc_rbtree_foreach(iter, rbtree)                                                    \
    for(void const *scc_pp_cat_expand(scc_rbtree_end_,__LINE__) =                           \
            (iter = scc_rbtree_impl_leftmost_value(rbtree),                                 \
                scc_rbtree_impl_iterstop(rbtree));                                          \
        iter != scc_pp_cat_expand(scc_rbtree_end_,__LINE__);                                \
        iter = scc_rbtree_impl_successor(iter))

//! .. _scc_rbtree_foreach_reversed:
//! .. c:macro:: scc_rbtree_foreach_reversed(iter, rbtree)
//!
//!     Reversed in-order iteration of the nodes in the rbtree.
//!
//!     .. seealso::
//!
//!         :ref:`scc_rbtree_foreach <scc_rbtree_foreach>`
//!
//!     :param iter:   Const-qualified pointer to the type of element stored in the tree
//!     :param rbtree: rbtree handle
#define scc_rbtree_foreach_reversed(iter, rbtree)                                           \
    for(void const *scc_pp_cat_expand(scc_rbtree_end_,__LINE__) =                           \
            (iter = scc_rbtree_impl_rightmost_value(rbtree),                                \
                scc_rbtree_impl_iterstop(rbtree));                                          \
        iter != scc_pp_cat_expand(scc_rbtree_end_,__LINE__);                                \
        iter = scc_rbtree_impl_predecessor(iter))

#endif /* SCC_RBTREE_H */
