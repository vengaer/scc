#ifndef SCC_HASHTAB_H
#define SCC_HASHTAB_H

#include "arch.h"
#include "bits.h"
#include "bug.h"
#include "canary.h"
#include "config.h"
#include "hash.h"
#include "mem.h"

#include <stddef.h>

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_hashtab:
 * \endverbatim
 *
 * Expands to an opaque pointer suitable for referring to a
 * ``hashtab`` storing instances of the provided \a type.
 *
 * \param type Type of the elements to store in the table
 *
 * The macros is used as the type of of a variable or parameter
 * declaration along the lines of
 *
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c
 *      :caption: Creating a ``hashtab`` storing ``int`` elements
 *
 *      scc_hashtab(int) tab;
 * \endverbatim
 */
#define scc_hashtab(type) type *

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_iter:
 * \endverbatim
 *
 * Expands to a dereferenceable type suitable for iterating
 * over the ``hashtab``.
 *
 * \param type The type stored in the ``hashtab``
 */
#define scc_hashtab_iter(type) type const *

#define SCC_HASHTAB_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

#define SCC_HASHTAB_CANARYSZ 32u

#ifndef SCC_HASHTAB_STACKCAP
/**
 * Default capacity of the stack buffer used for small-size optimization.
 *
 * Users may override this value when using the library by providing a
 * preprocessor definition with this name before including the header.
 *
 * \warning Must be a power of 2 greater than 16
 */
#define SCC_HASHTAB_STACKCAP 32
#endif

#if SCC_HASHTAB_STACKCAP < 32
#error Stack capacity must be at least 32
#endif

#if !scc_bits_is_power_of_2(SCC_HASHTAB_STACKCAP)
#error Stack capacity must be a power of 2
#endif

/**
 * Signature of the function used for compating elements in a ``hashtab``.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashtab/comparators.c
 *      :caption: Example ``int`` comparator.
 *      :start-at: int_eq
 *      :end-at: }
 *      :language: c
 *
 *  .. literalinclude:: /../examples/hashtab/comparators.c
 *      :caption: Example ``string`` comparator.
 *      :start-at: str_eq
 *      :end-at: }
 *      :language: c
 *
 * \endverbatim
 */
typedef _Bool(*scc_hashtab_eq)(void const *, void const *);

/**
 * Signature of the hash function used to hash keys in ``hashtab`` instances.
 *
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashtab/hash_functions.c
 *      :caption: Example FNV-1a hash function
 *      :start-at: fnv1a_64
 *      :end-before: /
 *      :language: c
 *
 *  .. literalinclude:: /../examples/hashtab/hash_functions.c
 *      :caption: Example FNV-1 hash function
 *      :start-at: fnv1_64
 *      :end-before: int main
 *      :language: c
 *
 * \endverbatim
 */
typedef scc_hash_type(*scc_hashtab_hash)(void const*, size_t);

typedef unsigned char scc_hashtab_metatype;

struct scc_hashtab_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

struct scc_hashtab_base {
    scc_hashtab_eq ht_eq;
    scc_hashtab_hash ht_hash;
    size_t ht_mdoff;
    size_t ht_size;
    size_t ht_capacity;
#ifdef SCC_PERFEVTS
    struct scc_hashtab_perfevts ht_perf;
#endif
    unsigned char ht_dynalloc;
    unsigned char ht_fwoff;
    unsigned char ht_buffer[];
};

#ifdef SCC_PERFEVTS
#define SCC_HASHTAB_INJECT_PERFEVTS(name)                                   \
    struct scc_hashtab_perfevts name;
#else
#define SCC_HASHTAB_INJECT_PERFEVTS(name)
#endif

#define scc_hashtab_impl_layout(type)                                       \
    struct {                                                                \
        struct {                                                            \
            struct {                                                        \
                scc_hashtab_eq ht_eq;                                       \
                scc_hashtab_hash ht_hash;                                   \
                size_t ht_mdoff;                                            \
                size_t ht_size;                                             \
                size_t ht_capacity;                                         \
                SCC_HASHTAB_INJECT_PERFEVTS(ht_perf)                        \
                unsigned char ht_dynalloc;                                  \
                unsigned char ht_fwoff;                                     \
                unsigned char ht_bkoff;                                     \
            } ht0;                                                          \
            type ht_curr;                                                   \
            type ht_data[SCC_HASHTAB_STACKCAP];                             \
        } ht1;                                                              \
        scc_hashtab_metatype ht_meta[SCC_HASHTAB_STACKCAP];                 \
        scc_hashtab_metatype ht_guard[SCC_HASHTAB_GUARDSZ];                 \
        SCC_CANARY_INJECT(SCC_HASHTAB_CANARYSZ)                             \
    }

#define scc_hashtab_impl_curroff(type)                                      \
    sizeof(                                                                 \
        struct {                                                            \
            struct {                                                        \
                scc_hashtab_eq ht_eq;                                       \
                scc_hashtab_hash ht_hash;                                   \
                size_t ht_mdoff;                                            \
                size_t ht_size;                                             \
                size_t ht_capacity;                                         \
                SCC_HASHTAB_INJECT_PERFEVTS(ht_perf)                        \
                unsigned char ht_dynalloc;                                  \
                unsigned char ht_fwoff;                                     \
                unsigned char ht_bkoff;                                     \
            } ht0;                                                          \
            type ht_curr[];                                                 \
        }                                                                   \
    )

#define scc_hashtab_impl_metaoff(type)                                      \
    sizeof(                                                                 \
        struct {                                                            \
            struct {                                                        \
                struct {                                                    \
                    scc_hashtab_eq ht_eq;                                   \
                    scc_hashtab_hash ht_hash;                               \
                    size_t ht_mdoff;                                        \
                    size_t ht_size;                                         \
                    size_t ht_capacity;                                     \
                    SCC_HASHTAB_INJECT_PERFEVTS(ht_perf)                    \
                    unsigned char ht_dynalloc;                              \
                    unsigned char ht_fwoff;                                 \
                    unsigned char ht_bkoff;                                 \
                } ht0;                                                      \
                type ht_curr;                                               \
                type ht_data[SCC_HASHTAB_STACKCAP];                         \
            } ht1;                                                          \
            scc_hashtab_metatype ht_meta[];                                 \
        }                                                                   \
    )

void *scc_hashtab_impl_new(struct scc_hashtab_base *base, size_t coff, size_t mdoff);

void *scc_hashtab_impl_new_dyn(scc_hashtab_eq eq, scc_hashtab_hash hash, size_t cap, size_t tabsz, size_t coff, size_t mdoff);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_with_hash:
 * \endverbatim
 *
 * Initialize a ``hashtab`` instance using the specified hash function.
 *
 * The macro expands to an opaque pointer suitable for referring to a ``hashtab`` storing \a type
 * instances. The constructed table has automatic storage duration.
 *
 * The call cannot fail.
 *
 * The returned pointer must be passed to
 * @verbatim embed:rst:inline :ref:`scc_hashtab_free <scc_hashtab_free>` @endverbatim
 * to ensure allocated memory is reclaimed.
 *
 * \param type The type to store in the ``hashtab``
 * \param eq Pointer to the function to use to compare keys
 * \param hash Pointer to the function to use for hashing keys
 *
 * \return Handle to a newly created ``hashtab``.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashtab/lifetime.c
 *      :caption: Initializing a ``hashtab`` using a custom FNV-1 hash function
 *      :start-at: main
 *      :end-before: OUTPUT
 *      :language: c
 * \endverbatim
 */
#define scc_hashtab_with_hash(type, eq, hash)                               \
    (type *)scc_hashtab_impl_new(                                           \
        (void *)&(scc_hashtab_impl_layout(type)) {                          \
            .ht1 = {                                                        \
                .ht0 = {                                                    \
                    .ht_eq = eq,                                            \
                    .ht_hash = hash,                                        \
                    .ht_capacity = SCC_HASHTAB_STACKCAP                     \
                },                                                          \
            },                                                              \
        },                                                                  \
        scc_hashtab_impl_curroff(type),                                     \
        scc_hashtab_impl_metaoff(type)                                      \
    )

/**
 * Like @verbatim embed:rst:inline :ref:`scc_hashtab_with_hash <scc_hashtab_with_hash>` @endverbatim
 * except for the ``hashtab`` being allocated on the heap rather than the stack.
 *
 * \note Unlike @verbatim embed:rst:inline :ref:`scc_hashtab_with_hash <scc_hashtab_with_hash>` @endverbatim,
 * ``scc_hashtab_with_hash_dyn``, as well as any scc function whose name ends with `dyn`, may fail. Comparing the
 * return value against ``NULL``is highly recommended.
 *
 * \param type Type of the elements to store in the ``hashtab``
 * \param eq Pointer to function to be used for key comparison
 * \param hash: Pointer to function to use for hashing keys
 *
 * \return Handle to a dynamically allocated ``hashtab``, or ``NULL`` on failure
 */
#define scc_hashtab_with_hash_dyn(type, eq, hash)                           \
    (type *)scc_hashtab_impl_new_dyn(                                       \
        eq,                                                                 \
        hash,                                                               \
        SCC_HASHTAB_STACKCAP,                                               \
        sizeof(scc_hashtab_impl_layout(type)),                              \
        scc_hashtab_impl_curroff(type),                                     \
        scc_hashtab_impl_metaoff(type)                                      \
    )

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_new:
 * \endverbatim
 *
 * Initializes a ``hashtab`` using the default
 * @verbatim embed:rst:inline :ref:`alternate Forwler-Noll-Vo <scc_hash_fnv1a_64>` @endverbatim
 * hash function.
 *
 * The macro is equivalent to invoking
 * @verbatim embed:rst:inline :ref:`scc_hashtab_with_hash <scc_hashtab_with_hash>` @endverbatim passing
 * the @verbatim embed:rst:inline :ref:`scc_hash_fnv1a_64 <scc_hash_fnv1a_64>` @endverbatim or
 * @verbatim embed:rst:inline :ref:`scc_hash_fnv1a_32 <scc_hash_fnv1a_32>` @endverbatim, depending on
 * the architecture.
 *
 * The call cannot fail.
 *
 *
 * \note The instantiated ``hashtab`` is allocated in the scope where the macro is invoked and should
 * therefore not be returned. Refer to
 * @verbatim embed:rst:inline :ref:`scc_hashtab_new_dyn <scc_hashtab_new_dyn>` @endverbatim for a dynamically
 * allocated ``hashtab``.
 *
 * \sa @verbatim embed:rst:inline :ref:`scc_hashtab_new_dyn <scc_hashtab_new_dyn>` @endverbatim for a
 *     dynamically allocated table.
 *
 * \param type Type of the elements to store in the ``hashtab``
 * \param eq Pointer used to compare keys for equality
 *
 * \return An opaque pointer referring to the newly created ``hashtab``
 */
#define scc_hashtab_new(type, eq)                                           \
    scc_hashtab_with_hash(type, eq, scc_hash_fnv1a)

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_new_dyn:
 * \endverbatim
 *
 * Like @verbatim embed:rst:inline :ref:`scc_hashtab_new <scc_hashtab_new>` @endverbatim
 * except that the returned ``hashtab`` is allocated on the heap rather than the stack.
 *
 * \note The call may fail in which case ``NULL`` is returned.
 *
 * \param type Type of the elements to store in the ``hashtab``
 * \param eq Pointer to function used to compare keys for equality
 *
 * \return Opaque pointer referring to a dynamically allocated ``hashtab`` or ``NULL`` on failure.
 */
#define scc_hashtab_new_dyn(type, eq)                                       \
    scc_hashtab_with_hash_dyn(type, eq, scc_hash_fnv1a)

inline size_t scc_hashtab_impl_bkpad(void const *tab) {
    return ((unsigned char const *)tab)[-1] + sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
}

#define scc_hashtab_impl_base_qual(tab, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(tab) - scc_hashtab_impl_bkpad(tab),          \
        struct scc_hashtab_base,                                            \
        ht_fwoff,                                                           \
        qual                                                                \
    )

#define scc_hashtab_impl_base(tab)                                          \
    scc_hashtab_impl_base_qual(tab,)

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_free:
 * \endverbatim
 *
 * Relaim memory used by the given ``hashtab``.
 *
 * The ``tab`` parameter must refer to a valid ``hashtab`` instance created
 * using one of the
 * @verbatim embed:rst:inline :ref:`initialization constructs <hashtab_init>` @endverbatim.
 *
 * \param tab Handle referring to the ``hashtab`` instance to free.
 */
void scc_hashtab_free(void *tab);

_Bool scc_hashtab_impl_insert(void *tabaddr, size_t elemsize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_insert:
 * \endverbatim
 *
 * Insert an element in the ``hashtab``.
 *
 * The insertion fails should the element in question already be present in the ``hashtab``.
 *
 * The call may result in the ``hashtab`` being rehashed. As such, any pointers referring to entries
 * in a ``hashtab```should be considered invalidated as soon as ``scc_hashtab_insert`` is called.
 *
 * While the return value indicates whether or not the pair was inserted, it provides no means of
 * determining whether or not the key was already present. If the latter is of interest, refer to
 * @verbatim embed:rst:inline :ref:`scc_hashtab_find <scc_hashtab_find>` @endverbatim.
 *
 * \note The \a tabaddr parameter takes the \b address of a handle returned by one of the
 *       @verbatim embed:rst:inline :ref:`initialization constructs <hashtab_init>` @endverbatim,
 *       \b not the handle itself.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashtab/insertion.c
 *      :caption: Example ``hashtab`` insertion
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param tabaddr Pointer to the ``hashtab`` handle.
 * \param value The value to insert.
 *
 * \return ``true`` if the insertion was successful, otherwise ``false``
 */
#define scc_hashtab_insert(tabaddr, value)                                  \
    scc_hashtab_impl_insert((**(tabaddr) = (value), (tabaddr)), sizeof(**(tabaddr)))

/**
 * Query the current capacity of the provided ``hashtab``
 *
 * \param tab The ``hashtab`` whose capacity is to be queried
 *
 * \return The current capacity of the ``hashtab``
 */
inline size_t scc_hashtab_capacity(void const *tab) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    return base->ht_capacity;
}

/**
 * Return the number of elements in the ``hashtab``
 *
 * \param tab Handle referring to the ``hashtab`` whose size is to be queried
 *
 * \return The number of elements in the ``hashtab``
 */
inline size_t scc_hashtab_size(void const *tab) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    return base->ht_size;
}

void const *scc_hashtab_impl_find(void const *tab, size_t elemsize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_find:
 * \endverbatim
 *
 * Probe for the given \a value
 *
 * \param tab ``hashtab`` handle
 * \param value The value to search for
 *
 * \return A ``const``-qualified pointer  to the value associated with the found element, or
 *         ``NULL`` if it wasn't found.
 */
#define scc_hashtab_find(tab, value)                                        \
    scc_hashtab_impl_find((*(tab) = (value), (tab)), sizeof(*(tab)))

_Bool scc_hashtab_impl_reserve(void *tabaddr, size_t capacity, size_t elemsize);

/**
 * Reserve storage for at least \a capacity number of elements in the ``hashtab``.
 *
 * If reallocation is required, any and all existing pointers into the table are
 * invalidated.
 *
 * \param tabaddr Address of the handle referring to the ``hashtab``
 * \param capacity The desired capacity
 *
 * \return ``true`` if enough memory could be reserved, otherwise ``false``
 */
#define scc_hashtab_reserve(tabaddr, capacity)                              \
    scc_hashtab_impl_reserve(tabaddr, capacity, sizeof(**(tabaddr)))

_Bool scc_hashtab_impl_remove(void *tab, size_t elemsize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashtab_remove:
 * \endverbatim
 *
 * Remove an element from the given ``hashtab``.
 *
 * \param tab Handle for identifying the ``hashtab``
 * \param value The element to remove
 *
 * \return ``true`` if the removal took place, ``false`` if the key was not found
 */
#define scc_hashtab_remove(tab, value)                                      \
    scc_hashtab_impl_remove((*(tab) = (value), (tab)), sizeof(*(tab)))

/**
 * Clear all entries in the given ``hashtab``
 *
 * \param tab Handle identifying the ``hashtab``
 */
void scc_hashtab_clear(void *tab);

/**
 * Clone the given ``hashtab``.
 *
 * The returned instance is allocated on the heap and contains the same elements
 * as the provided \a tab.
 *
 * \param tab Handle identifying the ``hashtab`` to clone
 *
 * \return Handle referring to the new ``hashtab``, or ``NULL`` on failure
 */
void *scc_hashtab_clone(void const *tab);

/**
 * Iterate over the ``hashtab``
 *
 * \note The order in which the elements are traversed is ultimately decided by
 *       the hash function used. If a specific order is required, users need to
 *       enforce this by crafting an appropriate hash function.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashtab/iteration.c
 *      :caption: Iterating over a ``hashtab``
 *      :start-after: int main
 *      :end-at: scc_hashtab_free
 *      :language: c
 *
 * \endverbatim
 *
 * \param iter An instance of @verbatim embed:rst:inline :ref:`scc_hashtab_iter <scc_hashtab_iter>` @endverbatim
 *             instantiated using the same type as the one stored in \a tab.
 * \param tab: Handle identifying the ``hashtab``
 */
#define scc_hashtab_foreach(iter, tab)                                      \
    for ((iter) = scc_hashtab_impl_iter_begin(tab, sizeof(*(tab)));         \
         (iter);                                                            \
         (iter) = scc_hashtab_impl_iter_next(tab, sizeof(*(tab)), iter))

void const *scc_hashtab_impl_iter_begin(void *tab, size_t elemsize);

void const *scc_hashtab_impl_iter_next(void *tab, size_t elemsize, void const *iter);

#endif /* SCC_HASHTAB_H */
