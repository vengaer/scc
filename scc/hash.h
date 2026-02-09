#ifndef SCC_HASH_H
#define SCC_HASH_H

#include <stddef.h>
#include <stdint.h>

#include <scc/config.h>
#include <scc/murmur32.h>
#include <scc/murmur64.h>

/**
 * 128 bit digest.
 *
 * Contains only a 16 byte array named digest.
 */
struct scc_digest128 {
    unsigned char digest[128u >> 3u];
};

#ifdef SCC_BITARCH_32
typedef uint_fast32_t scc_hash_type;
typedef int_fast32_t scc_hash_stype;
#else
typedef uint_fast64_t scc_hash_type;
typedef int_fast64_t scc_hash_stype;
#endif

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hash_fnv1a_32:
 * \endverbatim
 *
 * 32-bit alternative Fowler-Noll-Vo hash function.
 *
 * \param data Pointer to the data to be hashed. The data is treated as a consecutive
 *             array of bytes. Potential padding in structs must therefore
 *             be explicitly initialized to avoid erratic hashing behavior.
 * \param size Number of bytes at the address referred to by \a data to hash
 *
 * \return Computed 32-bit hash
 */
uint_fast32_t scc_hash_fnv1a_32(void const *data, size_t size);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hash_fnv1a_64:
 * \endverbatim
 *
 *
 * 64-bit alternative Fowler-Noll-Vo hash function.
 *
 * \param data Pointer to the data to be hashed. The data is treated as a consecutive
 *             array of bytes. Potential padding in structs must therefore
 *             be explicitly initialized to avoid erratic hashing behavior.
 * \param size Number of bytes at the address referred to by \a data to hash
 *
 * \return Computed 64-bit hash
 */
uint_fast64_t scc_hash_fnv1a_64(void const *data, size_t size);

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_hash_fnv1a:
 * \endverbatim
 *
 * Architecture-dependent Fowler-Noll-Vo hash function. On 32-bit architectures, the call
 * resolves to @verbatim embed:rst:inline :ref:`scc_hash_fnv1a_32 <scc_hash_fnv1a_32>` @endverbatim, otherwise
 * @verbatim embed:rst:inline :ref:`scc_hash_fnv1a_64 <scc_hash_fnv1a_64>` @endverbatim
 *
 * \param data Pointer to the data to be hashed. The data is treated as a consecutive
 *             array of bytes. Potential padding in structs must therefore
 *             be explicitly initialized to avoid erratic hashing behavior.
 * \param size Number of bytes at the address referred to by \a data to hash
 *
 * \return Computed hash
 */
inline scc_hash_type scc_hash_fnv1a(void const *data, size_t size) {
#ifdef SCC_BITARCH_32
    return scc_hash_fnv1a_32(data, size);
#else
    return scc_hash_fnv1a_64(data, size);
#endif
}


#if defined SCC_HAVE_UINT32_T || defined SCC_HAVE_UINT64_t
/**
 * 128-bit murmur3 hash
 *
 * \param digest Structure to store the 128 bit digest in.
 * \param data Data to calculate the hash over.
 * \param size Number of bytes to calculate the hash over.
 * \param seed Seed to pass to the implementation
 */
inline void scc_hash_murmur128(struct scc_digest128 *digest, void const *data,
        size_t size, uint_fast32_t seed)
{
#if defined SCC_HAVE_UINT64_t && defined SCC_BITARCH_64
    scc_murmur64_128(digest, data, size, seed);
#else
    scc_murmur32_128(digest, data, size, seed);
#endif
}

#endif


#endif /* SCC_HASH_H */
