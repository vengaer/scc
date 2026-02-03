#ifndef SCC_HASHMAP_H
#define SCC_HASHMAP_H

#include "arch.h"
#include "bits.h"
#include "bug.h"
#include "canary.h"
#include "config.h"
#include "hash.h"
#include "mem.h"
#include "perf.h"

#include <stddef.h>

#define scc_hashmap_impl_pair(keytype, valuetype)                                       \
    struct { keytype hp_key; valuetype hp_val; }

#define scc_hashmap_impl_pair_valoff(keytype, valuetype)                                \
    scc_align(sizeof(keytype), scc_alignof(valuetype))

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_hashmap:
 * \endverbatim
 *
 * Expands to an opaque pointer suitable for referring to a
 * ``hashmap`` mapping instances of the provided \a key \a type
 * to \a value \a type instances.
 *
 * \param keytype Type of the keys to store in the map
 * \param valuetype Type of the values to store in the map
 *
 * The macros is used as the type of of a variable or parameter
 * declaration along the lines of
 *
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c
 *      :caption: Creating a ``hashmap`` mapping ``char const *`` instances to ``short`` dittos.
 *
 *      scc_hashmap(char const *, short) map;
 * \endverbatim
 */
#define scc_hashmap(keytype, valuetype)                                                 \
    scc_hashmap_impl_pair(keytype, valuetype) *

#define SCC_HASHMAP_GUARDSZ ((unsigned)SCC_VECSIZE - 1u)

#define SCC_HASHMAP_CANARYSZ 32u

#define SCC_HASHMAP_DUPLICATE (~(~0ull >> 1u))

#ifndef SCC_HASHMAP_STACKCAP

/**
 * Default capacity of the stack buffer used for small-size optimization.
 *
 * Users may override this value when using the library by providing a
 * preprocessor definition with this name before including the header.
 *
 * \warning Must be a power of 2 greater than 16
 */
#define SCC_HASHMAP_STACKCAP 32
#endif

#if SCC_HASHMAP_STACKCAP < 32
#error Stack capacity must be at least 32
#endif

#if !scc_bits_is_power_of_2(SCC_HASHMAP_STACKCAP)
#error Stack capacity must be a power of 2
#endif

/**
 * Signature of the function used for compating keys in a ``hashmap``.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashmap/comparators.c
 *      :caption: Example ``int`` comparator.
 *      :start-at: int_eq
 *      :end-at: }
 *      :language: c
 *
 *  .. literalinclude:: /../examples/hashmap/comparators.c
 *      :caption: Example ``string`` comparator.
 *      :start-at: str_eq
 *      :end-at: }
 *      :language: c
 *
 * \endverbatim
 */
typedef _Bool(*scc_hashmap_eq)(void const *, void const *);

/**
 * Signature of the hash function used to hash keys in ``hashmap`` instances.
 *
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashmap/hash_functions.c
 *      :caption: Example FNV-1a hash function
 *      :start-at: fnv1a_64
 *      :end-before: /
 *      :language: c
 *
 *  .. literalinclude:: /../examples/hashmap/hash_functions.c
 *      :caption: Example FNV-1 hash function
 *      :start-at: fnv1_64
 *      :end-before: int main
 *      :language: c
 *
 * \endverbatim
 */
typedef scc_hash_type(*scc_hashmap_hash)(void const *, size_t);

typedef unsigned char scc_hashmap_metatype;

struct scc_hashmap_perfevts {
    size_t ev_n_rehashes;
    size_t ev_n_eqs;
    size_t ev_n_hash;
    size_t ev_n_inserts;
    size_t ev_bytesz;
};

struct scc_hashmap_base {
    scc_hashmap_eq hm_eq;
    scc_hashmap_hash hm_hash;
    size_t hm_valoff;
    size_t hm_mdoff;
    size_t hm_size;
    size_t hm_capacity;
    size_t hm_pairsize;
#ifdef SCC_PERFEVTS
    struct scc_hashmap_perfevts hm_perf;
#endif
    unsigned short hm_keyalign;
    unsigned short hm_valalign;
    unsigned char hm_dynalloc;
    unsigned char hm_valpad;
    unsigned char hm_fwoff;
    unsigned char hm_buffer[];
};

#ifdef SCC_PERFEVTS
#define SCC_HASHMAP_INJECT_PERFEVTS(name)                                   \
    struct scc_hashmap_perfevts name;
#else
#define SCC_HASHMAP_INJECT_PERFEVTS(name)
#endif

#define scc_hashmap_impl_layout(keytype, valuetype)                                         \
    struct {                                                                                \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    scc_hashmap_eq hm_eq;                                                   \
                    scc_hashmap_hash hm_hash;                                               \
                    size_t hm_valoff;                                                       \
                    size_t hm_mdoff;                                                        \
                    size_t hm_size;                                                         \
                    size_t hm_capacity;                                                     \
                    size_t hm_pairsize;                                                     \
                    SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                    \
                    unsigned short hm_keyalign;                                             \
                    unsigned short hm_valalign;                                             \
                    unsigned char hm_dynalloc;                                              \
                    unsigned char hm_valpad;                                                \
                    unsigned char hm_fwoff;                                                 \
                    unsigned char hm_bkoff;                                                 \
                } hm0;                                                                      \
                scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                          \
                keytype hm_keys[SCC_HASHMAP_STACKCAP];                                      \
            } hm1;                                                                          \
            valuetype hm_vals[SCC_HASHMAP_STACKCAP];                                        \
        } hm2;                                                                              \
        scc_hashmap_metatype hm_meta[SCC_HASHMAP_STACKCAP];                                 \
        scc_hashmap_metatype hm_guard[SCC_HASHMAP_GUARDSZ];                                 \
        SCC_CANARY_INJECT(SCC_HASHMAP_CANARYSZ)                                             \
    }

#define scc_hashmap_impl_curroff(keytype, valuetype)                                        \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                scc_hashmap_eq hm_eq;                                                       \
                scc_hashmap_hash hm_hash;                                                   \
                size_t hm_valoff;                                                           \
                size_t hm_mdoff;                                                            \
                size_t hm_size;                                                             \
                size_t hm_capacity;                                                         \
                size_t hm_pairsize;                                                         \
                SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                        \
                unsigned short hm_keyalign;                                                 \
                unsigned short hm_valalign;                                                 \
                unsigned char hm_dynalloc;                                                  \
                unsigned char hm_valpad;                                                    \
                unsigned char hm_fwoff;                                                     \
                unsigned char hm_bkoff;                                                     \
            } hm0;                                                                          \
            scc_hashmap_impl_pair(keytype, valuetype) hm_curr[];                            \
        }                                                                                   \
    )

#define scc_hashmap_impl_valoff(keytype, valuetype)                                         \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    scc_hashmap_eq hm_eq;                                                   \
                    scc_hashmap_hash hm_hash;                                               \
                    size_t hm_valoff;                                                       \
                    size_t hm_mdoff;                                                        \
                    size_t hm_size;                                                         \
                    size_t hm_capacity;                                                     \
                    size_t hm_pairsize;                                                     \
                    SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                    \
                    unsigned short hm_keyalign;                                             \
                    unsigned short hm_valalign;                                             \
                    unsigned char hm_dynalloc;                                              \
                    unsigned char hm_valpad;                                                \
                    unsigned char hm_fwoff;                                                 \
                    unsigned char hm_bkoff;                                                 \
                } hm0;                                                                      \
                scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                          \
                keytype hm_keys[SCC_HASHMAP_STACKCAP];                                      \
            } hm1;                                                                          \
            valuetype hm_vals[];                                                            \
        }                                                                                   \
    )

#define scc_hashmap_impl_mdoff(keytype, valuetype)                                          \
    sizeof(                                                                                 \
        struct {                                                                            \
            struct {                                                                        \
                struct {                                                                    \
                    struct {                                                                \
                        scc_hashmap_eq hm_eq;                                               \
                        scc_hashmap_hash hm_hash;                                           \
                        size_t hm_valoff;                                                   \
                        size_t hm_mdoff;                                                    \
                        size_t hm_size;                                                     \
                        size_t hm_capacity;                                                 \
                        size_t hm_pairsize;                                                 \
                        SCC_HASHMAP_INJECT_PERFEVTS(hm_perf)                                \
                        unsigned short hm_keyalign;                                         \
                        unsigned short hm_valalign;                                         \
                        unsigned char hm_dynalloc;                                          \
                        unsigned char hm_valpad;                                            \
                        unsigned char hm_fwoff;                                             \
                        unsigned char hm_bkoff;                                             \
                    } hm0;                                                                  \
                    scc_hashmap_impl_pair(keytype, valuetype) hm_curr;                      \
                    keytype hm_keys[SCC_HASHMAP_STACKCAP];                                  \
                } hm1;                                                                      \
                valuetype hm_vals[SCC_HASHMAP_STACKCAP];                                    \
            } hm2;                                                                          \
            scc_hashmap_metatype hm_meta[];                                                 \
        }                                                                                   \
    )

void *scc_hashmap_impl_new(struct scc_hashmap_base *base, size_t coff, size_t valoff, size_t keysize);

void *scc_hashmap_impl_new_dyn(struct scc_hashmap_base const *sbase, size_t mapsize, size_t coff, size_t valoff, size_t keysize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashmap_with_hash:
 * \endverbatim
 *
 * Initialize a ``hashmap`` instance using the specified hash function.
 *
 * The macro expands to an opaque pointer suitable for referring to a ``hashmap`` mapping instances
 * of \a key \a type to \a value \a type dittos. The constructed instance has automatic storage duration.
 *
 * The call cannot fail.
 *
 * The returned pointer must be passed to
 * @verbatim embed:rst:inline :ref:`scc_hashmap_free <scc_hashmap_free>` @endverbatim
 * to ensure allocated memory is reclaimed.
 *
 * \param keytype The type of the keys to store in the map
 * \param valuetype The type of the values to store in the map
 * \param eq Pointer to the function to use to compare keys
 * \param hash Pointer to the function to use for hashing keys
 *
 * \return Handle to a newly created ``hashmap``.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashmap/lifetime.c
 *      :caption: Initializing a ``hashmap`` using a custom FNV-1 hash function
 *      :start-after: main
 *      :end-at: Map now invalid
 *      :language: c
 * \endverbatim
 */
#define scc_hashmap_with_hash(keytype, valuetype, eq, hash)                                 \
    scc_hashmap_impl_new(                                                                   \
        (void *)&(scc_hashmap_impl_layout(keytype, valuetype)){                             \
            .hm2 = {                                                                        \
                .hm1 = {                                                                    \
                    .hm0 = {                                                                \
                        .hm_eq = eq,                                                        \
                        .hm_hash = hash,                                                    \
                        .hm_valoff = scc_hashmap_impl_valoff(keytype, valuetype),           \
                        .hm_mdoff = scc_hashmap_impl_mdoff(keytype, valuetype),             \
                        .hm_capacity = SCC_HASHMAP_STACKCAP,                                \
                        .hm_pairsize = sizeof(scc_hashmap_impl_pair(keytype, valuetype)),   \
                        .hm_keyalign = scc_alignof(keytype),                                \
                        .hm_valalign = scc_alignof(valuetype)                               \
                    },                                                                      \
                },                                                                          \
            },                                                                              \
        },                                                                                  \
        scc_hashmap_impl_curroff(keytype, valuetype),                                       \
        scc_hashmap_impl_pair_valoff(keytype, valuetype),                                   \
        sizeof(keytype)                                                                     \
    )

/**
 * Like @verbatim embed:rst:inline :ref:`scc_hashmap_with_hash <scc_hashmap_with_hash>` @endverbatim
 * except for the ``hashmap`` being allocated on the heap rather than the stack.
 *
 * \note Unlike @verbatim embed:rst:inline :ref:`scc_hashmap_with_hash <scc_hashmap_with_hash>` @endverbatim,
 * ``scc_hashmap_with_hash_dyn``, as well as any scc function whose name ends with `dyn`, may fail. Comparing the
 * return value against ``NULL``is highly recommended.
 *
 * \param keytype Type of the keys to be stored in the map
 * \param valuetype Type of the values to be stored in the map
 * \param eq Pointer to function to be used for key comparison
 * \param hash: Pointer to function to use for hashing keys
 *
 * \return Handle to a dynamically allocated ``hashmap``, or ``NULL`` on failure
 */
#define scc_hashmap_with_hash_dyn(keytype, valuetype, eq, hash)                             \
    scc_hashmap_impl_new_dyn(                                                               \
        (void *)&(struct scc_hashmap_base){                                                 \
            .hm_eq = eq,                                                                    \
            .hm_hash = hash,                                                                \
            .hm_valoff = scc_hashmap_impl_valoff(keytype, valuetype),                       \
            .hm_mdoff = scc_hashmap_impl_mdoff(keytype, valuetype),                         \
            .hm_capacity = SCC_HASHMAP_STACKCAP,                                            \
            .hm_pairsize = sizeof(scc_hashmap_impl_pair(keytype, valuetype)),               \
            .hm_keyalign = scc_alignof(keytype),                                            \
            .hm_valalign = scc_alignof(valuetype)                                           \
        },                                                                                  \
        sizeof(scc_hashmap_impl_layout(keytype, valuetype)),                                \
        scc_hashmap_impl_curroff(keytype, valuetype),                                       \
        scc_hashmap_impl_pair_valoff(keytype, valuetype),                                   \
        sizeof(keytype)                                                                     \
    )

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashmap_new:
 * \endverbatim
 *
 * Initializes a ``hashmap`` using the default
 * @verbatim embed:rst:inline :ref:`alternate Forwler-Noll-Vo <scc_hash_fnv1a_64>` @endverbatim
 * hash function.
 *
 * The macro is equivalent to invoking
 * @verbatim embed:rst:inline :ref:`scc_hashmap_with_hash <scc_hashmap_with_hash>` @endverbatim passing
 * the @verbatim embed:rst:inline :ref:`scc_hash_fnv1a_64 <scc_hash_fnv1a_64>` @endverbatim or
 * @verbatim embed:rst:inline :ref:`scc_hash_fnv1a_32 <scc_hash_fnv1a_32>` @endverbatim, depending on
 * the architecture.
 *
 * The call cannot fail.
 *
 * \note The instantiated ``hashmap`` is allocated in the scope where the macro is invoked and should
 * therefore not be returned. Refer to
 * @verbatim embed:rst:inline :ref:`scc_hashmap_new_dyn <scc_hashmap_new_dyn>` @endverbatim for a dynamically
 * allocated ``hashmap``.
 *
 * \param keytype Type of the keys to be stored in the ``hashmap``
 * \param valuetype Type of the values to be stored in the map
 * \param eq Pointer used to compare keys for equality
 *
 * \return An opaque pointer referring to the newly created ``hashmap``
 */
#define scc_hashmap_new(keytype, valuetype, eq)                                            \
    scc_hashmap_with_hash(keytype, valuetype, eq, scc_hash_fnv1a)

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashmap_new_dyn:
 * \endverbatim
 *
 * Like @verbatim embed:rst:inline :ref:`scc_hashmap_new <scc_hashmap_new>` @endverbatim
 * except that the returned ``hashmap`` is allocated on the heap rather than the stack.
 *
 * \note The call may fail in which case ``NULL`` is returned.
 *
 * \param keytype Type of the keys to store in the ``hashmap``
 * \param valuetype Type of the values to store in the map
 * \param eq Pointer to function used to compare keys for equality
 *
 * \return Opaque pointer referring to a dynamically allocated ``hashmap`` or ``NULL`` on failure.
 */
#define scc_hashmap_new_dyn(keytype, valuetype, eq)                                       \
    scc_hashmap_with_hash_dyn(keytype, valuetype, eq, scc_hash_fnv1a)

inline size_t scc_hashmap_impl_bkpad(void const *map) {
    return ((unsigned char const *)map)[-1] + sizeof(((struct scc_hashmap_base *)0)->hm_fwoff);
}

#define scc_hashmap_impl_base_qual(map, qual)                               \
    scc_container_qual(                                                     \
        (unsigned char qual *)(map) - scc_hashmap_impl_bkpad(map),          \
        struct scc_hashmap_base,                                            \
        hm_fwoff,                                                           \
        qual                                                                \
    )

#define scc_hashmap_impl_base(map)                                          \
    scc_hashmap_impl_base_qual(map,)

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashmap_free:
 * \endverbatim
 *
 * Relaim memory used by the given ``hashmap``.
 *
 * The ``map`` parameter must refer to a valid ``hashmap`` instance.
 *
 * \param map Handle referring to the ``hashmap`` instance to free.
 */
void scc_hashmap_free(void *map);

_Bool scc_hashmap_impl_insert(void *mapaddr, size_t keysize, size_t valsize);

/**
 * Insert a key-value pair in the ``hashmap``.
 *
 * If the \a key is already present in the ``hashmap``, its assocaited value is replaced with the
 * given \a value. If \a key is not found in the table, an entirely new pair is inserted.
 *
 * The call may result in the ``hashmap`` being rehashed. As such, any pointers referring to entries
 * in a ``hashmap```should be considered invalidated as soon as ``scc_hashmap_insert`` is called.
 *
 * While the return value indicates whether or not the pair was inserted, it provides no means of
 * determining whether or not the key was already present. If the latter is of interest, refer to
 * @verbatim embed:rst:inline :ref:`scc_hashmap_find <scc_hashmap_find>` @endverbatim.
 *
 * \note The \a mapaddr parameter takes the \b address of a handle returned by one of the
 *       @verbatim embed:rst:inline :ref:`initialization constructs <hashmap_init>` @endverbatim,
 *       \b not the handle itself.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashmap/insertion.c
 *      :caption: Example ``hashmap`` insertion
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param mapaddr Pointer to the ``hashmap`` handle.
 * \param key The key to insert.
 * \param value The value to insert.
 *
 * \return ``true`` if the insertion was successful, otherwise ``false``
 */
#define scc_hashmap_insert(mapaddr, key, value)                             \
    scc_hashmap_impl_insert((                                               \
            (*(mapaddr))->hp_key = (key),                                   \
            (*(mapaddr))->hp_val = (value),                                 \
            (mapaddr)                                                       \
        ),                                                                  \
        sizeof((*(mapaddr))->hp_key),                                       \
        sizeof((*(mapaddr))->hp_val)                                        \
    )

/**
 * Query the current capacity of the provided ``hashmap``
 *
 * \param map The ``hashmap`` whose capacity is to be queried
 *
 * \return The current capacity of the ``hashmap``
 */
inline size_t scc_hashmap_capacity(void const *map) {
    struct scc_hashmap_base const *base =
        scc_hashmap_impl_base_qual(map, const);
    return base->hm_capacity;
}

/**
 * Return the number of elements in the ``hashmap``
 *
 * The returned number indicates the number of key-values pairs in the ``hashmap``.
 *
 * \param map Handle referring to the ``hashmap`` whose size is to be queried
 *
 * \return The number of elements in the ``hashmap``
 */
inline size_t scc_hashmap_size(void const *map) {
    struct scc_hashmap_base const *base =
        scc_hashmap_impl_base_qual(map, const);
    return base->hm_size;
}

void *scc_hashmap_impl_find(void *map, size_t keysize, size_t valsize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashmap_find:
 * \endverbatim
 *
 * Probe for value identified by the given \a key.
 *
 * Provided that the key was found in the ``hashmap``, the associated value
 * may be modified through the returned pointer.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashmap/find_modify.c
 *      :caption: Modify value in ``hashmap`` by lookup
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param map ``hashmap`` handle
 * \param key The key to search for
 *
 * \return Pointer to the value associated with the provided \a key, or
 *         ``NULL`` if the key was not found
 */
#define scc_hashmap_find(map, key)                                      \
    scc_hashmap_impl_find(                                              \
        ((map)->hp_key = (key), (map)),                                 \
        sizeof((map)->hp_key),                                          \
        sizeof((map)->hp_val)                                           \
    )

_Bool scc_hashmap_impl_remove(void *map, size_t keysize);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hashmap_remove:
 * \endverbatim
 *
 * Remove a key-value pair from the given \a map.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/hashmap/removal.c
 *      :caption: Modify value in ``hashmap`` by lookup
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param map Handle for identifying the ``hashmap``
 * \param key The key to remove
 *
 * \return ``true`` if the removal took place, ``false`` if the key was not found
 */
#define scc_hashmap_remove(map, key)                                    \
    scc_hashmap_impl_remove(((map)->hp_key = (key), (map)), sizeof((map)->hp_key))

/**
 * Clear all entries in the given ``hashmap``
 *
 * \param map Handle identifying the ``hashmap``
 */
void scc_hashmap_clear(void *map);

/**
 * Clone the given ``hashmap``.
 *
 * The returned instance is allocated on the heap and contains the same key-value
 * pairs as the provided \a map.
 *
 * \param map Handle identifying the ``hashmap`` to clone
 *
 * \return Handle referring to the new ``hashmap``, or ``NULL`` on failure
 */
void *scc_hashmap_clone(void const *map);

#endif /* SCC_HASHMAP_H */
