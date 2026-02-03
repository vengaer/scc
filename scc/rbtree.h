#ifndef SCC_RBTREE_H
#define SCC_RBTREE_H

#include "arena.h"
#include "mem.h"
#include "pp_token.h"

#include <stddef.h>

/**
 * Expands to a type suitable for referring to an ``rbmap`` mapping \a keytype to \a valuetype
 *
 * \param type The type to store in the red-black tree
 */
#define scc_rbtree(type) type *

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbtree_iter:
 * \endverbatim
 *
 * Expands to a type suitable for iterating an ``rbtree`` containing
 * \a type instances
 *
 * \param type The type stored in the ``rbtree``
 */
#define scc_rbtree_iter(type) type const *

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbcompare:
 *  \endverbatim
 *
 * Signature of the function used for comparing keys in a ``rbtree``.
 *
 * <table>
 *  <caption id="rbcmp_rv">Return value</caption>
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
typedef int(*scc_rbcompare)(void const *, void const *);

enum scc_rbcolor {
    scc_rbcolor_black,
    scc_rbcolor_red
};

enum scc_rbdir {
    scc_rbdir_left,
    scc_rbdir_right
};

#define scc_rbnode_link_qual(node, idx, qual)                       \
    (*(struct scc_rbnode_base *qual *)                              \
        ((unsigned char qual *)&(node)->rn_left +                   \
            (scc_rbnode_link_offset(node)) * (idx)))

#define scc_rbnode_link(node, idx)                                  \
    scc_rbnode_link_qual(node, idx,)

#define scc_rbnode_value_qual(base, node, qual)                     \
    ((void qual *)(((unsigned char qual *)node) +                   \
        ((base)->rb_dataoff)))

#define scc_rbnode_value(base, node)                                \
    scc_rbnode_value_qual(base, node,)

struct scc_rbnode_base {
    struct scc_rbnode_base *rn_left;
    struct scc_rbnode_base *rn_right;
    enum scc_rbcolor rn_color;
    unsigned char rn_flags;
    unsigned char rn_data[];
};

struct scc_rbsentinel {
    struct scc_rbnode_base *rs_left;
    struct scc_rbnode_base *rs_right;
    enum scc_rbcolor rs_color;
    unsigned char rs_flags;
};

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

void *scc_rbtree_impl_new(struct scc_rbtree_base *base, size_t coff);

void *scc_rbtree_impl_new_dyn(size_t treesz, struct scc_arena *arena, scc_rbcompare compare, size_t coff, size_t dataoff);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbtree_new:
 * \endverbatim
 *
 * Create an ``rbtree`` instance storing elements of the provided \a type.
 *
 * The call cannot fail.
 *
 * The instance is allocated in the scope where the macro is invoked and should therefore
 * not be returned.
 *
 * \sa @verbatim embed:rst:inline :ref:`scc_rbtree_new_dyn <scc_rbtree_new_dyn>` @endverbatim
 *     for the counterpart relying on dynamic allocation.
 *
 * \param type Type of the elements to be stored in the ``rbtree``
 * \param compare Pointer to the comparison function to use. The signature should match
 *                @verbatim embed:rst:inline :ref:`scc_rbcompare <scc_rbcompare>` @endverbatim
 *
 * \return An opaque pointer referring to an ``rbtree`` allocated in the frame of the calling function
 */
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

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbtree_new_dyn:
 * \endverbatim
 *
 * Like @verbatim embed:rst:inline :ref:`scc_rbtree_new <scc_rbtree_new>` @endverbatim except for
 * the ``rbtree`` being allocated on the heap rather than the stack.
 *
 * \note Unlike @verbatim embed:rst:inline :ref:`scc_rbtree_new <scc_rbtree_new>` @endverbatim,
 *       ``scc_rbtree_new_dyn`` may fail. If it does, ``NULL`` is returned.
 *
 * \param type Type of the elements stored in the ``rbtree``
 * \param compare Pointer to the comparison function to use. The signature should match
 *                @verbatim embed:rst:inline :ref:`scc_rmcompare <scc_rmcompare>` @endverbatim
 *
 * \return An opaque pointer referring to a dynamically allocated``rbtree``, or ``NULL`` on failure.
 */
#define scc_rbtree_new_dyn(type, compare)                                                   \
    (type *)scc_rbtree_impl_new_dyn(                                                        \
        sizeof(scc_rbtree_impl_layout(type)),                                               \
        &scc_arena_new(scc_rbnode_impl_layout(type)),                                       \
        compare,                                                                            \
        scc_rbtree_impl_curroff(type),                                                      \
        scc_rbnode_impl_valoff(type)                                                        \
    )

inline size_t scc_rbtree_impl_npad(void const *rbtree) {
    return ((unsigned char const *)rbtree)[-1] + sizeof(unsigned char);
}

inline size_t scc_rbnode_link_offset(struct scc_rbnode_base const *node) {
    return ((unsigned char const*)&node->rn_right) - ((unsigned char const *)&node->rn_left);
}

inline _Bool scc_rbnode_thread(struct scc_rbnode_base const *node, enum scc_rbdir dir) {
    return node->rn_flags & (1u << dir);
}

inline size_t scc_rbnode_bkoff(void const *iter) {
    return ((unsigned char const *)iter)[-1];
}

#define scc_rbnode_impl_base_qual(valaddr, qual)                    \
    scc_container_qual(                                             \
        (unsigned char qual *)valaddr -                             \
            scc_rbnode_bkoff(valaddr),                              \
        struct scc_rbnode_base,                                     \
        rn_data,                                                    \
        qual                                                        \
    )

#define scc_rbnode_impl_base(valaddr)                               \
    scc_rbnode_impl_base_qual(valaddr,)


#define scc_rbtree_impl_base_qual(rbtree, qual)                                             \
    scc_container_qual(                                                                     \
        (unsigned char qual *)(rbtree) - scc_rbtree_impl_npad(rbtree),                      \
        struct scc_rbtree_base,                                                             \
        rb_fwoff,                                                                           \
        qual                                                                                \
    )

#define scc_rbtree_impl_base(rbtree)                                                        \
    scc_rbtree_impl_base_qual(rbtree,)

/**
 * Query the size of the given ``rbtree``.
 *
 * \param rbtree Handle referring to the ``rbtree``
 *
 * \return Number of elements stored in the ``rbtree``
 */
inline size_t scc_rbtree_size(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    return base->rb_size;
}

/**
 * Determine whether or not the given ``rbtree`` is empty.
 *
 * \param rbtree Handle referring to the ``rbtree``
 *
 * \return ``true`` if the ``rbtree`` is empty, ``false`` if it's not
 */
inline _Bool scc_rbtree_empty(void const *rbtree) {
    return !scc_rbtree_size(rbtree);
}

/**
 * Remove all elements from the given ``rbtree``.
 *
 * \param rbtree Handle identifying the ``rbtree`` to clear
 */
void scc_rbtree_clear(void *rbtree);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbtree_free:
 * \endverbatim
 *
 * Reclaim memory allocated for the provided ``rbtree``.
 *
 * The parameter must refer to a valid ``rbtree`` instantiated using on of
 * the ``rbtree`` @verbatim embed:rst:inline :ref:`initialization constructs <rbtree_init>` @endverbatim.
 *
 * \param rbtree Handle referring to the ``rbtree`` to free.
 */
void scc_rbtree_free(void *rbtree);

void *scc_rbtree_impl_generic_insert(void *rbtreeaddr, size_t elemsize);

inline _Bool scc_rbtree_impl_insert(void *rbtreeaddr, size_t elemsize) {
    return scc_rbtree_impl_generic_insert(rbtreeaddr, elemsize) == *(void **)rbtreeaddr;
}

/**
 * Insert the given element into the ``rbtree``.
 *
 * The \a value parameter must necessarily be the same type of that with which
 * the ``rbtree`` was instantiated. If it's not, it's subject to implicit conversion.
 *
 * \note ``scc_rbtree_insert`` takes a \b pointer to the handle returned by one of the
 * @verbatim embed:rst:inline :ref:`initialization constructs <rbtree_init>` @endverbatim,
 * \b not the handle itself.
 *
 * \param rbtreeaddr Address of the ``rbtree`` handle
 * \param value The element to insert
 *
 * \return ``true`` if the insertion was successful, otherwise ``false``.
 */
#define scc_rbtree_insert(rbtreeaddr, value)                                                \
    scc_rbtree_impl_insert((**(rbtreeaddr) = (value), rbtreeaddr), sizeof(**(rbtreeaddr)))

void const *scc_rbtree_impl_find(void const *rbtree);

/**
 * Look up an element in the ``rbtree``.
 *
 * If the element is found, a ``const``-qualified pointer referring to the element in the tree is returned.
 *
 * The \a value parameter must not necessarily be the same type as the one
 * with which the ``rbtree`` was instantiated. If it is not, it is implicitly converted
 * to the type stored in the instance.
 *
 *
 * \param rbtree Handle identifying the ``rbtree``
 * \param value The element to search for
 *
 * \return A pointer to the element in the tree, or ``NULL`` if it's not found.
 */
#define scc_rbtree_find(rbtree, value)                                                      \
    scc_rbtree_impl_find((*(rbtree) = (value), (rbtree)))

_Bool scc_rbtree_impl_remove(void *rbtree, size_t elemsize);

/**
 * Remove element from the ``rbtree``
 *
 * The \a value parameter must not necessarily be the same type as the one with which the
 * ``rbtree`` was instantiated. If it is not, the value is implicitly converted to the type
 * stored in the tree.
 *
 * \param rbtree Handle identifying the ``rbtree``
 * \param value The element to be removed
 *
 * \return ``true`` if the element was removed, otherwise ``false``
 */
#define scc_rbtree_remove(rbtree, value)                                                    \
    scc_rbtree_impl_remove((*(rbtree) = (value), (rbtree)), sizeof(*(rbtree)))

void const *scc_rbtree_impl_leftmost_value(void const *rbtree);

void const *scc_rbtree_impl_rightmost_value(void const *rbtree);

void const *scc_rbtree_impl_successor(void const *iter);

void const *scc_rbtree_impl_predecessor(void const *iter);

inline void const *scc_rbtree_impl_iterstop(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    return (unsigned char const *)&base->rb_sentinel + base->rb_dataoff;
}

void *scc_rbtree_impl_clone(void const *rbtree, size_t elemsize);

/**
 * Clone the given ``rbtree`` instance.
 *
 * The new instance is allocated on the heap and guaranteed to contain identical key-value pairs as the original.
 *
 * \param rbtree Handle identifying the ``rbtree`` to clone
 *
 * \return Handle to a dynamically allocated copy of the original ``rbtree``, or ``NULL`` on failure
 */
#define scc_rbtree_clone(rbtree)                                                            \
    scc_rbtree_impl_clone(rbtree, sizeof(*(rbtree)))

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbtree_foreach:
 * \endverbatim
 *
 * Iterate over the nodes in the ``rbtree``
 *
 * Nodes are traversed in the order defined by the
 * @verbatim embed:rst:inline :ref:comparator <scc_rbcompare>` @endverbatim
 * used.
 *
 * The macro expands to a scope executed with the \a iter - an instance whose type is
 * created using @verbatim embed:rst:inline :ref:`scc_rbtree_iter <scc_rbtree_iter>` @endverbatim -
 * referring to each element in the tree.
 *
 * The element is accessed by dereferencing the \a iter parameter.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/rbtree/tree_iteration.c
 *      :caption: Example of iterating over elements in an ``rbtree``
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param iter An instance of a type generated using
 *              @verbatim embed:rst:inline :ref:`scc_rbtree_iter <scc_rbtree_iter>` @endverbatim.
 *              Used as iteration variable
 * \param rbmap Handle identifying the ``rbtree``
 */
#define scc_rbtree_foreach(iter, rbtree)                                                    \
    for(void const *scc_pp_cat_expand(scc_rbtree_end_,__LINE__) =                           \
            (iter = scc_rbtree_impl_leftmost_value(rbtree),                                 \
                scc_rbtree_impl_iterstop(rbtree));                                          \
        iter != scc_pp_cat_expand(scc_rbtree_end_,__LINE__);                                \
        iter = scc_rbtree_impl_successor(iter))

/**
 * Like @verbatim embed:rst:inline :ref:`scc_rbtree_foreach <scc_rbtree_foreach>` @endverbatim
 * except that the pairs are visited in reversed order.
 *
 * \param iter An instance of a type generated using
 *              @verbatim embed:rst:inline :ref:`scc_rbtree_iter <scc_rbtree_iter>` @endverbatim.
 *              Used as iteration variable
 * \param rbtree Handle identifying the ``rbtree``
 */
#define scc_rbtree_foreach_reversed(iter, rbtree)                                           \
    for(void const *scc_pp_cat_expand(scc_rbtree_end_,__LINE__) =                           \
            (iter = scc_rbtree_impl_rightmost_value(rbtree),                                \
                scc_rbtree_impl_iterstop(rbtree));                                          \
        iter != scc_pp_cat_expand(scc_rbtree_end_,__LINE__);                                \
        iter = scc_rbtree_impl_predecessor(iter))

#endif /* SCC_RBTREE_H */
