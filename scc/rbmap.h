#ifndef SCC_RBMAP_H
#define SCC_RBMAP_H

#include "rbtree.h"

#include <stddef.h>

#define scc_rbmap_impl_pair(keytype, valuetype)                                             \
    struct { keytype rm_key; valuetype rm_value; }

/**
 * Expands to a type suitable for referring to an ``rbmap`` mapping \a keytype to \a valuetype
 *
 * \param keytype Type of the keys stored in the ``rbmap``
 * \param valuetype Type of the values stored in the ``rbmap``
 */
#define scc_rbmap(keytype, valuetype)                                                       \
    scc_rbmap_impl_pair(keytype, valuetype) *

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbmap_iter:
 * \endverbatim
 *
 * Expands to a type suitable for iterating an ``rbmap`` mapping
 * \a keytype instances to \a valuetype dittos.
 *
 * The keys and values may be accessed through the ``key`` and
 * ``value`` members in this type.
 *
 * \param keytype The type of the keys stored in the ``rbmap``
 * \param valuetype The type of the values stored in the ``rbmap``
 */
#define scc_rbmap_iter(keytype, valuetype)                                                  \
    struct { keytype const key; valuetype value; } *

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rmcompare:
 *  \endverbatim
 *
 * Signature of the function used for comparing keys in a ``rbmap``.
 *
 * <table>
 *  <caption id="rbmcmp_rv">Return value</caption>
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
typedef int(*scc_rmcompare)(void const *, void const *);

#define scc_rbmnode_impl_layout(keytype, valuetype) \
    struct {                                                                                \
        struct {                                                                            \
            struct scc_rbnode_base *rn_left;                                                \
            struct scc_rbnode_base *rn_right;                                               \
            enum scc_rbcolor rn_color;                                                      \
            unsigned char rn_flags;                                                         \
            unsigned char rn_bkoff;                                                         \
        } rbmn0;                                                                            \
        scc_rbmap_impl_pair(keytype, valuetype) rn_pair;                                    \
    }

#define scc_rbmnode_impl_pairoff(keytype, valuetype)                                        \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct scc_rbnode_base *rn_left;                                            \
                struct scc_rbnode_base *rn_right;                                           \
                enum scc_rbcolor rn_color;                                                  \
                unsigned char rn_flags;                                                     \
                unsigned char rn_bkoff;                                                     \
            } rbmn0;                                                                        \
            scc_rbmap_impl_pair(keytype, valuetype) rn_pair[];                              \
        }                                                                                   \
    )

#define scc_rbmap_impl_layout(keytype, valuetype)                                           \
    struct {                                                                                \
        struct {                                                                            \
            unsigned short const rm_dataoff;                                                \
            size_t rm_size;                                                                 \
            scc_rmcompare rm_compare;                                                       \
            struct scc_arena rm_arena;                                                      \
            struct scc_rbsentinel rm_sentinel;                                              \
            unsigned char rm_fwoff;                                                         \
            unsigned char rm_bkoff;                                                         \
        } rbm0;                                                                             \
        scc_rbmap_impl_pair(keytype, valuetype) rm_curr;                                    \
    }

#define scc_rbmap_impl_curroff(keytype, valuetype)                                          \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                unsigned short const rm_dataoff;                                            \
                size_t rm_size;                                                             \
                scc_rmcompare rm_compare;                                                   \
                struct scc_arena rm_arena;                                                  \
                struct scc_rbsentinel rm_sentinel;                                          \
                unsigned char rm_fwoff;                                                     \
                unsigned char rm_bkoff;                                                     \
            } rbm0;                                                                         \
            scc_rbmap_impl_pair(keytype, valuetype) rm_curr[];                              \
        }                                                                                   \
    )

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbmap_new:
 * \endverbatim
 *
 * Create an ``rbmap`` instance mapping keys of type \a keytype to values of type \a valuetype.
 *
 * The call cannot fail.
 *
 * The instance is allocated in the scope where the macro is invoked and should therefore
 * not be returned.
 *
 * \sa @verbatim embed:rst:inline :ref:`scc_rbmap_new_dyn <scc_rbmap_new_dyn>` @endverbatim
 *     for the counterpart relying on dynamic allocation.
 *
 * \param keytype Type of the keys to be stored in the ``rbmap``
 * \param valuetype Type of the values to be stored in the ``rbmap``
 * \param compare Pointer to the comparison function to use. The signature should match
 *                @verbatim embed:rst:inline :ref:`scc_rmcompare <scc_rmcompare>` @endverbatim
 *
 * \return An opaque pointer referring to an ``rbmap`` allocated in the frame of the calling function
 */
#define scc_rbmap_new(keytype, valuetype, compare)                                          \
    scc_rbtree_impl_new(                                                                    \
        (void *)&(scc_rbmap_impl_layout(keytype, valuetype)) {                              \
            .rbm0 = {                                                                       \
                .rm_dataoff = scc_rbmnode_impl_pairoff(keytype, valuetype),                 \
                .rm_compare = compare,                                                      \
                .rm_arena = scc_arena_new(scc_rbmnode_impl_layout(keytype, valuetype)),     \
            },                                                                              \
        },                                                                                  \
        scc_rbmap_impl_curroff(keytype, valuetype)                                          \
   )

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbmap_new_dyn:
 * \endverbatim
 *
 * Like @verbatim embed:rst:inline :ref:`scc_rbmap_new <scc_rbmap_new>` @endverbatim except for
 * the ``rbmap`` being allocated on the heap rather than the stack.
 *
 * \note Unlike @verbatim embed:rst:inline :ref:`scc_rbmap_new <scc_rbmap_new>` @endverbatim,
 *       ``scc_rbmap_new_dyn`` may fail. If it does, ``NULL`` is returned.
 *
 * \param keytype Type of the keys stored in the ``rbmap``
 * \param valuetype Type of the values stored in the ``rbmap``
 * \param compare Pointer to the comparison function to use. The signature should match
 *                @verbatim embed:rst:inline :ref:`scc_rmcompare <scc_rmcompare>` @endverbatim
 *
 * \return An opaque pointer referring to a dynamically allocated``rbmap``, or ``NULL`` on failure.
 */
#define scc_rbmap_new_dyn(keytype, valuetype, compare)                                      \
    scc_rbtree_impl_new_dyn(                                                                \
        sizeof(scc_rbmap_impl_layout(keytype, valuetype)),                                  \
        &scc_arena_new(scc_rbmnode_impl_layout(keytype, valuetype)),                        \
        compare,                                                                            \
        scc_rbmap_impl_curroff(keytype, valuetype),                                         \
        scc_rbmnode_impl_pairoff(keytype, valuetype)                                        \
    )

/**
 * Query the size of the given ``rbmap``.
 *
 * \param map Handle referring to the ``rbmap``
 *
 * \return Number of key-value pairs stored in the ``rbmap``
 */
inline size_t scc_rbmap_size(void const *map) {
    return scc_rbtree_size(map);
}

/**
 * Determine whether or not the given ``rbmap`` is empty.
 *
 * \param map Handle referring to the ``rbmap``
 *
 * \return ``true`` if the ``rbmap`` is empty, ``false`` if it's not
 */
inline _Bool scc_rbmap_empty(void const *map) {
    return !scc_rbmap_size(map);
}

/**
 * Remove all key-value pairs from the given ``rbmap``.
 *
 * \param map Handle identifying the ``rbmap`` to clear
 */
inline void scc_rbmap_clear(void *map) {
    scc_rbtree_clear(map);
}

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbmap_free:
 * \endverbatim
 *
 * Reclaim memory allocated for the provided ``rbmap``.
 *
 * The parameter must refer to a valid ``rbmap`` instantiated using on of
 * the ``rbmap`` @verbatim embed:rst:inline :ref:`initialization constructs <rbmap_init>` @endverbatim.
 *
 * \param map Handle referring to the ``rbmap`` to free.
 */
inline void scc_rbmap_free(void *map) {
    scc_rbtree_free(map);
}

_Bool scc_rbmap_impl_insert(void *rbmapaddr, size_t elemsize, size_t valoff);

/**
 * Insert the given key-value pair into the ``rbmap``.
 *
 * If the key is already present in the map, its corresponding value is updated to the given one.
 *
 * Neither the \a key nor \a value parameters must necessarily be the same type of those with which
 * the ``rbmap`` was instantiated. If they are not, they are subject to implicit conversion.
 *
 * \note ``scc_rbmap_insert`` takes a \b pointer to the handle returned by one of the
 * @verbatim embed:rst:inline :ref:`initialization constructs <rbmap_init>` @endverbatim,
 * \b not the handle itself.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/rbmap/insertion.c
 *      :caption: Insert the pairs {0, 0}, {1, 2}, {2, 4} and {3, 6} in an ``rbmap``
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param mapaddr Address of the ``rbmap`` handle
 * \param key The key part of the pair to insert
 * \param value The value to insert
 *
 * \return ``true`` if the insertion was successful, otherwise ``false``.
 */
#define scc_rbmap_insert(mapaddr, key, value)                                               \
    scc_rbmap_impl_insert((                                                                 \
            (*(mapaddr))->rm_key = (key),                                                   \
            (*(mapaddr))->rm_value = (value),                                               \
            (mapaddr)                                                                       \
        ),                                                                                  \
        sizeof(**(mapaddr)),                                                                \
        ((unsigned char const *)&(*(mapaddr))->rm_value -                                   \
            (unsigned char const *)&(*(mapaddr))->rm_key)                                   \
    )

void *scc_rbmap_impl_find(void *map, size_t valoff);

/**
 * Look up a value associated with the provided \a key in an ``rbmap``.
 *
 * If the key is found, the address of it's associated value is returned. This pointer
 * may be used to modify the value within the ``rbmap``.
 *
 * The \a key parameter must not necessarily be the same type as the one
 * with which the ``rbmap`` was instantiated. If it is not, it is implicitly converted
 * to the type stored in the instance.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/rbmap/modify_value.c
 *      :caption: Modify value in ``rbmap`` through looked-up pointer
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param map Handle identifying the ``rbmap``
 * \param key The key to search for
 *
 * \return A pointer to the value associated with the provided \a key, or ``NULL``
 *         if the key is not found.
 */
#define scc_rbmap_find(map, key)                                                            \
    scc_rbmap_impl_find(                                                                    \
        ((map)->rm_key = (key), (map)),                                                     \
        ((unsigned char const *)&(map)->rm_value - (unsigned char const *)&(map)->rm_key)   \
    )

/**
 * Remove pair identified by the supplied \a key
 *
 * The \a key parameter must not necessarily be the same type as the one with which the
 * ``rbmap`` was instantiated. If it is not, the value is implicitly converted to the type
 * stored in the map.
 *
 * \param map Handle identifying the ``rbmap``
 * \param key Key identifying the pair to be removed
 *
 * \return ``true`` if the key-value pair was removed, otherwise ``false``
 */
#define scc_rbmap_remove(map, key)                                                          \
    scc_rbtree_impl_remove(                                                                 \
        ((map)->rm_key = (key), (map)),                                                     \
        sizeof(*(map))                                                                      \
    )

void *scc_rbmap_impl_leftmost_pair(void *map);

void *scc_rbmap_impl_rightmost_pair(void *map);

void *scc_rbmap_impl_successor(void *iter);

void *scc_rbmap_impl_predecessor(void *iter);

inline void const *scc_rbmap_impl_iterstop(void const *map) {
    return scc_rbtree_impl_iterstop(map);
}

/**
 * Clone the given ``rbmap`` instance.
 *
 * The new instance is allocated on the heap and guaranteed to contain identical key-value pairs as the original.
 *
 * \param map Handle identifying the ``rbmap`` to clone
 *
 * \return Handle to a dynamically allocated copy of the original ``rbmap``, or ``NULL`` on failure
 */
#define scc_rbmap_clone(map)                                                                \
    scc_rbtree_impl_clone(map, sizeof(*(map)))

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_rbmap_foreach:
 * \endverbatim
 * Iterate over the nodes in the ``rbmap``
 *
 * Nodes are traversed in the order defined by the
 * @verbatim embed:rst:inline :ref:comparator <scc_rmcompare>` @endverbatim
 * used.
 *
 * The macro expands to a scope executed with the \a iter - an instance whose type is
 * created using @verbatim embed:rst:inline :ref:`scc_rbmap_iter <scc_rbmap_iter>` @endverbatim -
 * referring to each pair in the map.
 *
 * The key is accessed through the a field named ``key``. the value through one named
 * ``value``.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/rbmap/forward_iteration.c
 *      :caption: Example of iterating over the pairs in an ``rbmap``
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param iter An instance of a type generated using
 *              @verbatim embed:rst:inline :ref:`scc_rbmap_iter <scc_rbmap_iter>` @endverbatim.
 *              Used as iteration variable
 * \param map Handle identifying the ``rbmap``
 */
#define scc_rbmap_foreach(iter, map)                                                        \
    for (void const *scc_pp_cat_expand(scc_rbmap_end_,__LINE__) =                            \
            ((iter) = scc_rbmap_impl_leftmost_pair(map),                                    \
                scc_rbmap_impl_iterstop(map));                                              \
        (iter) != scc_pp_cat_expand(scc_rbmap_end_,__LINE__);                               \
        (iter) = scc_rbmap_impl_successor(iter))

/**
 * Like @verbatim embed:rst:inline :ref:`scc_rbmap_foreach <scc_rbmap_foreach>` @endverbatim
 * except that the pairs are visited in reversed order.
 *
 * \param iter An instance of a type generated using
 *              @verbatim embed:rst:inline :ref:`scc_rbmap_iter <scc_rbmap_iter>` @endverbatim.
 *              Used as iteration variable
 * \param map Handle identifying the ``rbmap``
 */
#define scc_rbmap_foreach_reversed(iter, rbmap)                                             \
    for (void const *scc_pp_cat_expand(scc_rbmap_end_,__LINE__) =                            \
            (iter = scc_rbmap_impl_rightmost_pair(rbmap),                                   \
                scc_rbmap_impl_iterstop(rbmap));                                            \
        iter != scc_pp_cat_expand(scc_rbmap_end_,__LINE__);                                 \
        iter = scc_rbmap_impl_predecessor(iter))

#endif /* SCC_RBMAP_H */
