#ifndef SCC_BTREE_H
#define SCC_BTREE_H

#include "arena.h"
#include "bits.h"
#include "bug.h"
#include "mem.h"

#include <stddef.h>

/**
 * Expands to a type suitable for storing a handle to a B-tree
 * containing the specified type.
 *
 * \param type The type to store in the ``btree``.
 */
#define scc_btree(type) type *

/**
 * Signature of the function used for comparing keys in a ``btree``.
 *
 * <table>
 *  <caption id="btcmp_rv">Return value</caption>
 *  <tr><th>Condition <th>Value
 *  <tr><td>left < right<td> < 0
 *  <tr><td>left == right<td> 0
 *  <tr><td>left > right<td> > 0
 * </table>
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/btree/comparators.c
 *      :caption: Example ``int`` comparator.
 *      :start-at: compare_int
 *      :end-at: }
 *      :language: c
 *
 * .. literalinclude:: /../examples/btree/comparators.c
 *      :caption: Example ``string`` comparator.
 *      :start-at: compare_str
 *      :end-at: }
 *      :language: c
 *
 * \endverbatim
 */
typedef int(*scc_btcompare)(void const *, void const *);

#ifndef SCC_BTREE_FAULT_ORDER
/**
 * Default order of ``btree`` instances.
 *
 * The value may be overridden by defining it before including
 * the header. Instances of a specific order may be instantiated
 * using ``scc_btree_with_order``.
 *
 * \note Choosing an even number is strongly advised as this allows
 * the implementation to balance the tree preemptively during modification.
 *
 * \warning Must be greater than 2. The implementation also requires that
 * it is smaller than ``SIZE_MAX / 2`` but choosing values even remotely this size
 * is ill-advised.
 */
#define SCC_BTREE_DEFAULT_ORDER 6u
#endif /* SCC_BTREE_DEFAULT_ORDER */

#if SCC_BTREE_DEFAULT_ORDER <= 2
#error Order must be at least 2
#endif

struct scc_btnode_base {
    unsigned char bt_flags;
    unsigned short bt_nkeys;
    unsigned char bt_data[];
};

struct scc_btree_base {
    unsigned short const bt_order;
    unsigned short const bt_dataoff;
    unsigned short const bt_linkoff;
    size_t bt_size;
    struct scc_btnode_base *bt_root;
    scc_btcompare bt_compare;
    struct scc_arena bt_arena;
    unsigned char bt_dynalloc;
    unsigned char bt_fwoff;
    unsigned char bt_data[];
};

#define scc_btnode_impl_layout(type, order)                                                     \
    struct {                                                                                    \
        struct {                                                                                \
            struct {                                                                            \
                unsigned char bt_flags;                                                         \
                unsigned short bt_nkeys;                                                        \
            } btn0;                                                                             \
            type bt_data[(order) - 1u];                                                         \
        } btn1;                                                                                 \
        struct scc_btnode_base *bt_links[order];                                                \
    }

#define scc_btnode_impl_dataoff(type)                                                           \
    sizeof(                                                                                     \
        struct {                                                                                \
            struct {                                                                            \
                unsigned char bt_flags;                                                         \
                unsigned short bt_nkeys;                                                        \
            } btn0;                                                                             \
            type bt_data[];                                                                     \
        }                                                                                       \
    )

#define scc_btnode_impl_linkoff(type, order)                                                    \
    sizeof(                                                                                     \
        struct {                                                                                \
            struct {                                                                            \
                struct {                                                                        \
                    unsigned char bt_flags;                                                     \
                    unsigned short bt_nkeys;                                                    \
                } btn0;                                                                         \
                type bt_data[(order) - 1u];                                                     \
            } btn1;                                                                             \
            struct scc_btnode_base *bt_links[];                                                 \
        }                                                                                       \
    )

#define scc_btree_impl_layout(type, order)                                                          \
    struct {                                                                                        \
        struct {                                                                                    \
            struct {                                                                                \
                unsigned short const bt_order;                                                      \
                unsigned short const bt_dataoff;                                                    \
                unsigned short const bt_linkoff;                                                    \
                size_t bt_size;                                                                     \
                struct scc_btnode_base *bt_root;                                                    \
                scc_btcompare bt_compare;                                                           \
                struct scc_arena bt_arena;                                                          \
                unsigned char bt_dynalloc;                                                          \
                unsigned char bt_fwoff;                                                             \
                unsigned char bt_bkoff;                                                             \
            } bt0;                                                                                  \
            type bt_curr;                                                                           \
        } bt1;                                                                                      \
        scc_btnode_impl_layout(type, order) bt_rootmem;                                             \
    }

#define scc_btree_impl_curroff(type)                                                                \
    sizeof(                                                                                         \
        struct {                                                                                    \
            struct {                                                                                \
                unsigned short const bt_order;                                                      \
                unsigned short const bt_dataoff;                                                    \
                unsigned short const bt_linkoff;                                                    \
                size_t bt_size;                                                                     \
                struct scc_btnode_base *bt_root;                                                    \
                scc_btcompare bt_compare;                                                           \
                struct scc_arena bt_arena;                                                          \
                unsigned char bt_dynalloc;                                                          \
                unsigned char bt_fwoff;                                                             \
                unsigned char bt_bkoff;                                                             \
            } bt0;                                                                                  \
            type bt_curr[];                                                                         \
        }                                                                                           \
    )

#define scc_btree_impl_rootoff(type, order)                                                         \
    sizeof(                                                                                         \
        struct {                                                                                    \
            struct {                                                                                \
                struct {                                                                            \
                    unsigned short const bt_order;                                                  \
                    unsigned short const bt_dataoff;                                                \
                    unsigned short const bt_linkoff;                                                \
                    size_t bt_size;                                                                 \
                    struct scc_btnode_base *bt_root;                                                \
                    scc_btcompare bt_compare;                                                       \
                    struct scc_arena bt_arena;                                                      \
                    unsigned char bt_dynalloc;                                                      \
                    unsigned char bt_fwoff;                                                         \
                    unsigned char bt_bkoff;                                                         \
                } bt0;                                                                              \
                type bt_curr;                                                                       \
            } bt1;                                                                                  \
            scc_btnode_impl_layout(type, order) bt_rootmem[];                                       \
        }                                                                                           \
    )


void *scc_btree_impl_new(void *base, size_t coff, size_t rootoff);

void *scc_btree_impl_new_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff);

inline void *scc_btree_impl_with_order(void *base, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btree_base *)base)->bt_order;
    if(order < 3u) {
        return 0;
    }
    return scc_btree_impl_new(base, coff, rootoff);
}

inline void *scc_btree_impl_with_order_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btree_base *)sbase)->bt_order;
    if(order < 3u) {
        return 0;
    }
    return scc_btree_impl_new_dyn(sbase, basesz, coff, rootoff);
}

/**
 * Instantiate a ``btree`` of \a order storing instances of \a type.
 *
 * The tree is constructed in the frame of the calling function.
 *
 * The provided \a compare function pointer is used to compare the keys. The map is constructed
 * in the frame of the function in which the macro is expanded.
 *
 * \param type      The type of the values to be stored in the ``btree``.
 * \param compare   Pointer to the comparison function to use. The signature should match ``scc_btcompare``.
 * \param order     The desired order of the ``btree``. Must be at least 2.
 *
 * \return          An opaque pointer to a ``btree``, or ``NULL`` if the order is invalid.
 */
#define scc_btree_with_order(type, compare, order)                                                  \
    scc_btree_impl_with_order(&(scc_btree_impl_layout(type, order)) {                               \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = order,                                                              \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, order),                             \
                    .bt_arena = scc_arena_new(scc_btnode_impl_layout(type, order)),                 \
                    .bt_compare = compare                                                           \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, order)                                                         \
    )

/**
 * Instantiate a ``btree`` storing instances of \a type.
 *
 * The tree uses the provided \a compare function and is created in the frame of
 * the calling function. The call is equivalent to invoking ``scc_btree_with_order``, passing
 * ``SCC_BTREE_DEFAULT_ORDER`` as the order parameter.
 *
 * The call cannot fail.
 *
 * \param type    The type to be stored in the ``btree``.
 * \param compare Pointer to a function used to compare entries. The signature should match
 *  ``scc_btcompare``.
 *
 *  \return An opaqe pointer to a ``btre`` allocated in the frame of the calling function.
 */
#define scc_btree_new(type, compare)                                                                \
    (type *)scc_btree_impl_new(&(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)) {            \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = SCC_BTREE_DEFAULT_ORDER,                                            \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, SCC_BTREE_DEFAULT_ORDER),           \
                    .bt_arena = scc_arena_new(                                                      \
                        scc_btnode_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)                       \
                    ),                                                                              \
                    .bt_compare = (compare)                                                         \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, SCC_BTREE_DEFAULT_ORDER)                                       \
    )

/**
 * Instantiate a dynamically allocated ``btree`` with the provided \a order.
 *
 * Effectively equivalent to ``scc_btree_with_order`` in every way except that the constructed
 * tree is placed on the heap.
 *
 * \warning The call may fail, in which case ``NULL`` is returned.
 *
 * \param type    The type of the values to be stored in the tree.
 * \param compare Pointer to a comparison function, the signature of which should match
 *  ``scc_btcompare``.
 *  \param order  The order of the tree. Using an even value is advised.
 *
 *  \return An opaque pointer to a ``btree`` allocated on the heap, of ``NULL`` on failure.
 */
#define scc_btree_with_order_dyn(type, compare, order)                                              \
    scc_btree_impl_with_order_dyn(&(scc_btree_impl_layout(type, order)) {                           \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = (order),                                                            \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, order),                             \
                    .bt_arena = scc_arena_new(scc_btnode_impl_layout(type, (order))),               \
                    .bt_compare = (compare)                                                         \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        sizeof(scc_btree_impl_layout(type, order)),                                                 \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, order)                                                         \
    )

/**
 * Instantiate a ``btree`` with default order and place it on the heap.
 *
 * The call if effectively similar to ``scc_btree_new`` with the exception that the tree
 * is placed on the heap instead of the stack.
 *
 * \warning The call may fail, in which case ``NULL`` is returned.
 *
 * \param type    The type to be stored in the tree
 * \param compare Pointer to a comparison function, the signature of which should match
 *  ``scc_btcompare``.
 *
 *  \return An opaque pointer to a ``btree`` allocated on the heap, or ``NULL`` on failure.
 */
#define scc_btree_new_dyn(type, compare)                                                            \
    (type *)scc_btree_impl_new_dyn(&(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)) {        \
            .bt1 = {                                                                                \
                .bt0 = {                                                                            \
                    .bt_order = SCC_BTREE_DEFAULT_ORDER,                                            \
                    .bt_dataoff = scc_btnode_impl_dataoff(type),                                    \
                    .bt_linkoff = scc_btnode_impl_linkoff(type, SCC_BTREE_DEFAULT_ORDER),           \
                    .bt_arena = scc_arena_new(                                                      \
                        scc_btnode_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)                       \
                    ),                                                                              \
                    .bt_compare = (compare)                                                         \
                },                                                                                  \
            },                                                                                      \
        },                                                                                          \
        sizeof(scc_btree_impl_layout(type, SCC_BTREE_DEFAULT_ORDER)),                               \
        scc_btree_impl_curroff(type),                                                               \
        scc_btree_impl_rootoff(type, SCC_BTREE_DEFAULT_ORDER)                                       \
    )

inline size_t scc_btree_impl_npad(void const *btree) {
    return ((unsigned char const *)btree)[-1] + sizeof(unsigned char);
}

#define scc_btree_impl_base_qual(btree, qual)                                                       \
    scc_container_qual(                                                                             \
        (unsigned char qual *)(btree) - scc_btree_impl_npad(btree),                                 \
        struct scc_btree_base,                                                                      \
        bt_fwoff,                                                                                   \
        qual                                                                                        \
    )

#define scc_btree_impl_base(btree)                                                                  \
    scc_btree_impl_base_qual(btree,)

/**
 * Reclaim memory allocated for the ``btree``.
 *
 * The parameter must refer to a valid B-tree returned by one of the
 * @verbatim embed:rst:inline :ref:`B-tree initialization <btree_init>` @endverbatim
 * constructs.
 *
 * \param btree Handle identifying the ``btree`` to free
 */
void scc_btree_free(void *btree);

/**
 * Return the order of the given ``btree``
 *
 * \param btree Handle identifying the ``btree``
 *
 * \return The order of the \a btree
 */
inline size_t scc_btree_order(void const *btree) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    return base->bt_order;
}

/**
 * Query the size of the given ``btree``
 *
 * \param btree Handle identifying the ``btree``
 *
 * \return Number of elements in the \a btree
 */
inline size_t scc_btree_size(void const *btree) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    return base->bt_size;
}

_Bool scc_btree_impl_insert(void *btreeaddr, size_t elemsize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_btree_insert:
 * \endverbatim
 *
 * Insert the given value into the specified ``btree``.
 *
 * The ``value`` parameter must not necessarily be the same type as the one
 * with which the ``btree`` was intantiated. If it is not, it is implicitly converted
 * to the value type of the ``btree``.
 *
 * \note The \a btreeaddr parameter should be the \b address of the handle used to refer to
 *       the ``btree``, \b not the handle itself.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/btree/insertion.c
 *      :caption: Example of ``btree`` insertion
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param btreeaddr Address of the handle used to refer to the ``btree``
 * \param value The value to insert
 *
 * \return ``true`` if the value was inserted, ``false`` on failure
 */
#define scc_btree_insert(btreeaddr, value)                                                              \
    scc_btree_impl_insert((**(btreeaddr) = (value), btreeaddr), sizeof(**(btreeaddr)))

void const *scc_btree_impl_find(void const *btree, size_t elemsize);

/**
 * Search for and, if found, return a pointer to, an element matching the given value.
 *
 * The \a value parameter must not necessarily be the same type as the one
 * with which the ``btree`` was instantiated. If it is not, the value is implicitly
 * converted to the type stored in the ``btree``.
 *
 * \note The ``btree`` may contain duplicates. No guarantee is given for which of
 *       the duplicates is returned.
 *
 * \param btree Handle referring to the ``btree``  instance
 * \param value The value to search for
 *
 * \return A pointer to one of the found elements, or ``NULL`` if none is found.
 */
#define scc_btree_find(btree, value)                                                                    \
    scc_btree_impl_find((*(btree) = (value), (btree)), sizeof(*(btree)))

_Bool scc_btree_impl_remove(void *btree, size_t elemsize);

/**
 * Find and remove the specified value in, and from, the given ``btree``.
 *
 * Should the ``btree`` contain several copies of the value in question, only one
 * instance is removed.
 *
 * The \a value parameter must not necessarily be the same type as the one
 * with which the ``btree`` was instantiated. If it is not, the value is implicitly
 * converted to the type stored in the ``btree``.
 *
 * \param btree Handle referring to the ``btree`` in question
 * \param value The value to remove
 *
 * \return ``true`` if a value was removed, ``false`` if no such value was found
 */
#define scc_btree_remove(btree, value)                                                                  \
    scc_btree_impl_remove(((*(btree) = (value)), (btree)), sizeof(*(btree)))

void *scc_btree_impl_clone(void const *btree, size_t elemsize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_btree_clone:
 * \endverbatim
 *
 * Clone the given ``btree``.
 *
 * The returned copy is allocated on the heap and contains the same elements as the
 * provided mould.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/btree/clone.c
 *      :caption: Example cloning a ``btree``
 *      :start-after: int main
 *      :end-at: scc_btree_free(copy
 *      :language: c
 *
 * \endverbatim
 *
 * \param btree Handle identifying the ``btree`` to clone
 *
 * \return An opaque handle referring to a new ``btree``, allocated on the heap. Or ``NULL``
 *         on failure
 */
#define scc_btree_clone(btree)                                                                              \
    scc_btree_impl_clone(btree, sizeof(*(btree)))

#endif /* SCC_BTREE_H */
