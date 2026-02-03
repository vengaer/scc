#ifndef SCC_HASH_H
#define SCC_HASH_H

#include <stddef.h>
#include <stdint.h>

#include <scc/config.h>

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


#endif /* SCC_HASH_H */
