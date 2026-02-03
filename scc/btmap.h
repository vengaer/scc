#ifndef SCC_BTMAP_H
#define SCC_BTMAP_H

#include "arena.h"
#include "btree.h"
#include "mem.h"

#include <stddef.h>

#define scc_btmap_impl_pair(keytype, valuetype)                                                                 \
    struct { keytype btm_key; valuetype btm_value; }

#define scc_btmap_impl_pair_valoff(keytype, valuetype)                                                          \
    scc_align(sizeof(keytype), scc_alignof(valuetype))

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_btmap:
 * \endverbatim
 *
 * Expands to an opaque pointer suitable for referring to a
 * ``btmap`` mapping instances of the provided \a key \a type
 * to \a value \a type instances.
 *
 * \param keytype Type of the keys to store in the map
 * \param valuetype Type of the values to store in the map
 *
 * The macro is used as the type of a variable or parameter
 * declaration along the lines of
 *
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c
 *      :caption: Creating a ``btmap`` mapping ``int`` instances to ``char const *`` dittos.
 *
 *      scc_btmap(int, char const *) map;
 * \endverbatim
 */
#define scc_btmap(keytype, valuetype)                                                                           \
    scc_btmap_impl_pair(keytype, valuetype) *

/**
 * Signature of the function used for comparing keys in a ``btmap``.
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
typedef scc_btcompare scc_btmcompare;

/**
 * Default order of ``btmap`` instances
 *
 * The value may be overridden by defining it before including
 * the header. Instances of a specific order may be instantiated
 * using ``scc_btmap_with_order``.
 *
 * \note Choosing an even number is strongly advised as this allows
 * the implementation to balance the tree preemptively during modification.
 *
 * \warning Must be greater than 2. The implementation also requires that
 * it is smaller than ``SIZE_MAX / 2`` but choosing values even remotely this size
 * is ill-advised.
 */
#ifndef SCC_BTMAP_DEFAULT_ORDER
#define SCC_BTMAP_DEFAULT_ORDER SCC_BTREE_DEFAULT_ORDER
#endif /* SCC_BTMAP_DEFAULT_ORDER */

struct scc_btmnode_base {
    unsigned char btm_flags;
    unsigned short btm_nkeys;
    unsigned char btm_data[];
};

struct scc_btmap_base {
    unsigned short const btm_order;
    unsigned short const btm_keyoff;
    unsigned short const btm_valoff;
    unsigned short const btm_linkoff;
    unsigned short const btm_keysize;
    unsigned short const btm_valsize;
    size_t btm_size;
    struct scc_btmnode_base *btm_root;
    scc_btmcompare btm_compare;
    struct scc_arena btm_arena;
    unsigned char const btm_kvoff;
    unsigned char btm_dynalloc;
    unsigned char btm_fwoff;
    unsigned char btm_data[];
};

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
                scc_btmcompare btm_compare;                                                                             \
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
                scc_btmcompare btm_compare;                                                                             \
                struct scc_arena btm_arena;                                                                             \
                unsigned char const btm_kvoff;                                                                          \
                unsigned char btm_dynalloc;                                                                             \
                unsigned char btm_fwoff;                                                                                \
                unsigned char btm_bkoff;                                                                                \
            } btmn0;                                                                                                    \
            scc_btmap_impl_pair(keytype, valuetype) btm_curr[];                                                         \
        }                                                                                                               \
    )

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
                    scc_btmcompare btm_compare;                                                                         \
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

/**
 * Instantiate a ``btmap`` of \a order, mapping instances of \a keytype to dittos of \a valuetype.
 *
 * The provided \a compare function pointer is used to compare the keys. The map is constructed
 * in the frame of the function in which the macro is expanded.
 *
 * \param keytype   The type of the keys to be stored in the ``btmap``.
 * \param valuetype The type of the values to be stored in the ``btmap``.
 * \param compare   Pointer to the comparison function to use.
 * \param order     The desired order of the ``btmap``. Must be at least 2.
 *
 * \return          An opaque pointer to a ``btmap``, or ``NULL`` if the order is invalid.
 */
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

/**
 * Instantiate a ``btmap`` of ``SCC_BTMAP_DEFAULT_ORDER`` order.
 *
 * The instance maps instances of \a keytype to \a valuetype, comparing keys using the
 * provided \a compare function.

 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/btmap/instantiation.c
 *      :caption: Instantiate a B-tree map.
 *      :start-at: Arbitrary
 *      :end-before: = OUTPUT
 *      :language: c
 *
 * \endverbatim
 *
 * \param keytype   The type of the keys stored in the ``btmap``.
 * \param valuetype The type of values stored in the ``btmap``.
 * \param compare   A pointer to the comparison function to use.
 *
 * \return An opaque pointer to a ``btmap`` allocated in the frame of the calling function.
 */
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

/**
 * Instantiate a ``btmap``, with the supplied order, placing it on the heap
 *
 * \warning The call may fail in which case ``NULL`` is returned.
 *
 * \param keytype The type of the keys to be stored in the ``btmap``.
 * \param valuetype The type of the values to be stored in the ``btmap``.
 * \param compare Pointer to the comparison function to use.
 * \param order Desired order of the ``btmap``. Must be at least 2.
 *
 * \return An opaque pointer to the ``btmap``, or ``NULL`` on failure.
 */
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

/**
 * Instantiate a ``btmap`` of order ``SCC_BTMAP_DEFAULT_ORDER`` and place it on the heap.
 *
 * \warning The call may fail in which case ``NULL`` is returned.
 *
 * \param keytype The type of the keys to be stored in the map.
 * \param valuetype The type of the values to be stored in the map.
 * \param compare Pointer to the comparison function to use.
 *
 * \return An opaque pointer to a ``btmap`` allocated on the heap, or ``NULL`` on allocation failure.
 */
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

void *scc_btmap_impl_new(void *base, size_t coff, size_t rootoff);

void *scc_btmap_impl_new_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff);

inline void *scc_btmap_impl_with_order(void *base, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btmap_base *)base)->btm_order;
    if(order < 3u) {
        return 0;
    }
    return scc_btmap_impl_new(base, coff, rootoff);
}

inline void *scc_btmap_impl_with_order_dyn(void *sbase, size_t basesz, size_t coff, size_t rootoff) {
    unsigned order = ((struct scc_btmap_base *)sbase)->btm_order;
    if(order < 3u) {
        return 0;
    }
    return scc_btmap_impl_new_dyn(sbase, basesz, coff, rootoff);
}

inline size_t scc_btmap_impl_npad(void const *btmap) {
    return ((unsigned char const *)btmap)[-1] + sizeof(unsigned char);
}

#define scc_btmap_impl_base_qual(btmap, qual)                                                       \
    scc_container_qual(                                                                             \
        (unsigned char qual *)(btmap) - scc_btmap_impl_npad(btmap),                                 \
        struct scc_btmap_base,                                                                      \
        btm_fwoff,                                                                                  \
        qual                                                                                        \
    )

#define scc_btmap_impl_base(btmap)                                                                  \
    scc_btmap_impl_base_qual(btmap,)

/**
 * Reclaim memory allocated for the ``btmap``
 *
 * The parameter must refer to a valid ``btmap``.
 *
 * \param btmap Handle to the ``btmap`` to free.
 */
void scc_btmap_free(void *btmap);

/**
 * Return the order of the given ``btmap``.
 *
 * \param btmap Handle to the map whose order is to be queried.
 *
 * \return The order of the ``btmap``.
 */
inline size_t scc_btmap_order(void const *btmap) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    return base->btm_order;
}

/**
 * Query the size of the given ``btmap``.
 *
 * \param btmap ``btmap`` handle.
 *
 * \return The size of the provided ``btmap``
 */
inline size_t scc_btmap_size(void const *btmap) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    return base->btm_size;
}

_Bool scc_btmap_impl_insert(void *btmapaddr);

/**
 * Insert the given key-value pair into the provided ``btmap``.
 *
 * If the key already exists, its associated value is overwritten.
 *
 * \warning The ``btmap`` may include a key at most 1 time. This is a stark
 *  contrast to the behavior of the ``btree``.
 *
 * The \a value parameter must not necessarily be the same type as the one with
 * which the ``btmap`` was instantiated. If it is not, the provided value is
 * implicitly converted to value type.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/btmap/insert.c
 *      :caption: Insert values in a ``btmap``.
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param btmapaddr Address of the ``btmap`` handle.
 * \param key The key to insert.
 * \param value The value to be inserted.
 *
 * \return ``true`` if the value was inserted, otherwise ``false``
 */
#define scc_btmap_insert(btmapaddr, key, value)                                                         \
    scc_btmap_impl_insert((                                                                             \
            (*(btmapaddr))->btm_key = (key),                                                            \
            (*(btmapaddr))->btm_value = (value),                                                        \
            (btmapaddr)                                                                                 \
        )                                                                                               \
    )

void *scc_btmap_impl_find(void *btmap);

/**
 * Look up value associated with the provided key.
 *
 * The \a key parameter must not necessarily be the same type as the key type
 * with which the ``btmap`` was instantiated. If it is not, the value is implicitly
 * converted to the key type.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/btmap/find.c
 *      :caption: Look up and modify values in a ``btmap``.
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param btmap Handle to the ``btmap``.
 * \param key The key whose associated value is to be looked up.
 *
 * \return A mutable pointer to the value associated with the provided key, or ``NULL`` on failure.
 */
#define scc_btmap_find(btmap, key)                                                                      \
    scc_btmap_impl_find((                                                                               \
            ((btmap)->btm_key = (key)), (btmap)                                                         \
        )                                                                                               \
    )

_Bool scc_btmap_impl_remove(void *btmap);

/**
 * Find and remove the key-value pair identified by the supplied key.
 *
 * The \a key parameter must not necessarily be the same type as the key type
 * with which the ``btmap`` was instantiated. If it is not, the key is implicitly
 * converted to the key type.
 *
 * \param btmap ``btmap`` handle.
 * \param key The key identifying  the key-value pair to remove.
 *
 * \return ``true`` if the pair was removed, or ``false`` if the key was not found.
 */
#define scc_btmap_remove(btmap, key)                                                                    \
    scc_btmap_impl_remove((                                                                             \
            ((btmap)->btm_key = (key)), (btmap)                                                         \
        )                                                                                               \
    )

/**
 * Clone the given ``btmap``, yielding a copy with the same size and elements.
 *
 * The new copy is allocated on the heap.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/btmap/clone.c
 *      :caption: Create a copy of a ``btmap``.
 *      :start-after: int main
 *      :end-at: scc_btmap_free
 *      :language: c
 *
 * \endverbatim
 *
 * \param btmap The ``btmap`` instance to clone.
 *
 * \return A copy of the provided ``btmap``, ``NULL`` on failure.
 */
void *scc_btmap_clone(void const *btmap);

#endif /* SCC_BTMAP_H */
