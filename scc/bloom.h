#ifndef SCC_BLOOM_H
#define SCC_BLOOM_H

#include <scc/config.h>
#include <scc/hash.h>
#include <scc/mem.h>

#include <stddef.h>
#include <stdint.h>

#if defined SCC_HAVE_UINT32_T || defined SCC_HAVE_UINT64_T

/**
 * Expands to an opaque pointer suitable for referring to a
 * bloom filter containing instance of the provided \a type.
 *
 * \param type The type to store in the filter.
 *
 * The macro is as the type of a variable or parameter
 * declaration along the lines of
 *
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c
 *      :caption: Creating a bloom filter for storing ``int`` instances.
 *
 *      scc_bloom(int) flt;
 * \endverbatim
 */
#define scc_bloom(type) type *

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_bloom_hash:
 * \endverbatim
 *
 * Signature of the function used to hash values in a bloom filter
 */
typedef void(*scc_bloom_hash)(struct scc_digest128 *, void const *, size_t, uint_fast32_t);

struct scc_bloom_base {
    scc_bloom_hash bm_hash;
    unsigned bm_nbits;
    unsigned bm_nhashes;
    unsigned char bm_tail[];
};

#define scc_bloom_impl_layout(type, m)                                  \
    struct {                                                            \
        struct {                                                        \
            scc_bloom_hash bm_hash;                                     \
            unsigned bm_nbits;                                          \
            unsigned bm_nhashes;                                        \
            unsigned char bm_npad;                                      \
            unsigned char bm_dynalloc;                                  \
        } bm_base;                                                      \
        type bm_tmp;                                                    \
        unsigned char bm_buckets[((m + 7u) & ~7u) >> 3u];                            \
    }

#define scc_bloom_impl_offset(type)                                     \
    sizeof(                                                             \
        struct {                                                        \
            struct {                                                    \
                scc_bloom_hash bm_hash;                                 \
                unsigned bm_nbits;                                      \
                unsigned bm_nhashes;                                    \
                unsigned char bm_npad;                                  \
                unsigned char bm_dynalloc;                              \
            } bm_base;                                                  \
            type bm_tmp[];                                              \
        }                                                              \
    )


/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_bloom_new:
 * \endverbatim
 *
 * Initialize a bloom filter tracking instances of the specified \a type.
 *
 * The resulting filter is placed in the frame of the function in which
 * the macro is involved.
 *
 * Regardless of size, users are responsible for destroying the
 * filter using ``scc_bloom_free``.
 *
 * The call is guaranteed to succeed.
 *
 * \note The \a m and \a k parameters should be greater than 0. If they are not,
 * they are defaulted to 8 and 4, respectively.
 *
 * \see @verbatim embed:rst:inline :ref:`scc_bloom_new_dyn <scc_bloom_new_dyn>` @endverbatim
 *      for version allocating the filter on the heap.
 *
 * \param type The type of the instances to be tracked in the filter.
 * \param m Size of the filter, in bits. Must be an integer constant expression.
 * \param k Number of hash functions to use.
 *
 * \return A handle to an instantiated filter
 */
#define scc_bloom_new(type, m, k)                                       \
    (type *)scc_bloom_impl_new(                                         \
        (void *)&(scc_bloom_impl_layout(type, m)) { 0 },                \
        scc_bloom_impl_offset(type),                                    \
        (m),                                                            \
        (k)                                                             \
    )

void *scc_bloom_impl_new(struct scc_bloom_base *base, size_t offset,
        unsigned m, unsigned k);

/**
 * Like @verbatim embed:rst:inline :ref:`scc_bloom_new <scc_bloom_new>` @endverbatim
 * but with support for a custom hash function.
 *
 * \param type The type of the instances to be tracked in the filter.
 * \param m Size of the filter, in bits. Must be an integer constant expression.
 * \param k Number of hash functions to use.
 * \param hash Pointer to hash function to use. Should be compatible with the
 *        the @verbatim embed:rst:inline :ref:`scc_bloom_hash <scc_bloom_hash>` @endverbatim
 *        typedef.
 *
 * \return A handle to an instantiated filter
 */
#define scc_bloom_with_hash(type, m, k, hash)                           \
    (type *)scc_bloom_impl_with_hash(                                   \
        (void *)&(scc_bloom_impl_layout(type, m)) { 0 },                \
        scc_bloom_impl_offset(type),                                    \
        (m),                                                            \
        (k),                                                            \
        (hash)                                                          \
    )

void *scc_bloom_impl_with_hash(struct scc_bloom_base *base, size_t offset,
        unsigned m, unsigned k, scc_bloom_hash hash);

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_bloom_new_dyn:
 * \endverbatim
 *
 * Like @verbatim embed:rst:inline :ref:`scc_bloom_new <scc_bloom_new>` @endverbatim
 * except that the resulting filter is allocated on the heap rather than
 * on the stack.
 *
 * Regardless of size, users are responsible for destroying the
 * filter using ``scc_bloom_free``.
 *
 * \note The call may fail, check the returned value against ``NULL``.
 *
 * \param type The type of the instances to be tracked in the filter.
 * \param m Size of the filter, in bits. Must be an integer constant expression.
 * \param k Number of hash functions to use
 *
 * \return A handle to an instantiated filter, or ``NULL`` on failure.
 */
#define scc_bloom_new_dyn(type, m, k)                                   \
    (type *)scc_bloom_impl_new_dyn(                                     \
        sizeof(scc_bloom_impl_layout(type, m)),                         \
        scc_bloom_impl_offset(type),                                    \
        (m),                                                            \
        (k)                                                             \
    )

void *scc_bloom_impl_new_dyn(size_t size, size_t offset, unsigned m,
        unsigned k);

/**
 * Like @verbatim embed:rst:inline :ref:scc_bloom_new_dyn <scc_bloom_new_dyn>` @endverbatim
 *
 * \param type The type of the instances to be tracked in the filter.
 * \param m Size of the filter, in bits. Must be an integer constant expression.
 * \param k Number of hash functions to use.
 * \param hash Pointer to hash function to use. Should be compatible with the
 *        the @verbatim embed:rst:inline :ref:`scc_bloom_hash <scc_bloom_hash>` @endverbatim
 *        typedef.
 *
 * \return A handle to an instantiated filter, or ``NULL`` on failure.
 */
#define scc_bloom_with_hash_dyn(type, m, k, hash)                       \
    (type *)scc_bloom_impl_with_hash_dyn(                               \
        sizeof(scc_bloom_impl_layout(type, m)),                         \
        scc_bloom_impl_offset(type),                                    \
        (m),                                                            \
        (k),                                                            \
        (hash)                                                          \
    )

void *scc_bloom_impl_with_hash_dyn(size_t size, size_t offset, unsigned m,
        unsigned k, scc_bloom_hash hash);

inline size_t scc_bloom_impl_npad(void const *flt) {
    return ((unsigned char const *)flt)[-2] + (sizeof(unsigned char) << 1u);
}

#define scc_bloom_impl_base_qual(flt, qual)                             \
    scc_container_qual(                                                 \
        (unsigned char qual *)(flt) - scc_bloom_impl_npad(flt),         \
        struct scc_bloom_base,                                          \
        bm_tail,                                                        \
        qual                                                            \
    )

#define scc_bloom_impl_base(flt)                                        \
    scc_bloom_impl_base_qual(flt,)

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_bloom_free:
 * \endverbatim
 *
 * Reclaim memory allocated for the given bloom filter.
 *
 * Used regardless of which method was used to create the
 * filter.
 *
 * \param flt Handle to the filter to free.
 */
void scc_bloom_free(void *flt);

/**
 * Insert the provided value in the bloom filter
 *
 * \param fltaddr Address of the handle referring to the filter in question
 * \param value THe value to insert
 */
#define scc_bloom_insert(fltaddr, value)                                    \
    scc_bloom_impl_insert(((void)(**(fltaddr) = value),*(fltaddr)), sizeof(**(fltaddr)))

void scc_bloom_impl_insert(void *flt, size_t elemsize);

/**
 * Test whether the provided value is in the set.
 *
 * \note False positives @verbatim embed:rst:inline **do** @endverbatim occur. False
 * negatives do not.
 *
 * \param flt The filter to be checked
 * \param value The value to look for
 *
 * \return ``true`` if the value may be in the filter, ``false`` if it definitely is not
 */
#define scc_bloom_test(flt, value)                                          \
    scc_bloom_impl_test(((void)(*(flt) = value),(flt)), sizeof(*(flt)))

_Bool scc_bloom_impl_test(void *flt, size_t elemsize);

/**
 * Get the size of the filter's bitset, in bytes.
 *
 * \param flt Bloom filter handle
 *
 * \return Size of the bitset, in bytes
 */
inline size_t scc_bloom_capacity(void const *flt) {
    struct scc_bloom_base const *base = scc_bloom_impl_base_qual(flt, const);
    return base->bm_nbits >> 3u;
}

/**
 * Get the number of hashes used by the filter
 *
 * \param flt Bloom filter handle
 *
 * \return Number of hashes used by the filter
 */
inline size_t scc_bloom_nhashes(void const *flt) {
    struct scc_bloom_base const *base = scc_bloom_impl_base_qual(flt, const);
    return base->bm_nhashes;
}

#ifdef SCC_HAVE_LIBM
/**
 * Return the approximate number of elements present in the set.
 *
 * \note Available only if the library was linked against libm.
 *
 * \param flt Handle referring to the bloom filter
 *
 * \return The approximate number of elements stored in the set.
 */
#define scc_bloom_size(flt)                                                 \
    scc_bloom_impl_size(flt, sizeof(*flt))

size_t scc_bloom_impl_size(void const *flt, size_t elemsize);
#endif /* SCC_HAVE_LIBM */

/**
 * Clone the provided bloom filter.
 *
 * The copy is allocated dynamically and completely separate from the
 * original. Both need to be passed to
 * @verbatim embed:rst:inline :ref:`scc_bloom_free <scc_bloom_free>` @endverbatim
 * once no longer required.
 *
 * \param flt The filter to copy
 *
 * \return A handle to the copy, or ``NULL`` on failure.
 */
#define scc_bloom_clone(flt)                                                \
    scc_bloom_impl_clone(flt, sizeof(*(flt)))

void *scc_bloom_impl_clone(void const *flt, size_t elemsize);

#endif  /* SCC_HAVE_UINT32_T || SCC_HAVE_UINT64_T */

#endif /* SCC_BLOOM_H */
