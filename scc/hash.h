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

//! .. c:function:: scc_hash_type scc_hash_fnv1a_32(void const *data, size_t size)
//!
//!     32-bit Fowler-Noll-Vo hash
//!
//!     :param data: Address of the data to be hashed. The data
//!                  is treated as a consecutive array of bytes.
//!                  Potential padding in structs must therefore
//!                  be explicitly initialized to avoid erratic
//!                  hashing behavior.
//!     :param size: The size of the data to be hashed, in bytes
//!     :returns: The 32-bit alternative Fowler-Noll-Vo hash of the given :c:texpr:`data`
uint_fast32_t scc_hash_fnv1a_32(void const *data, size_t size);

//! .. c:function:: scc_hash_type scc_hash_fnv1a_64(void const *data, size_t size)
//!
//!     64-bit Fowler-Noll-Vo hash
//!
//!     :param data: Address of the data to be hashed. The data
//!                  is treated as a consecutive array of bytes.
//!                  Potential padding in structs must therefore
//!                  be explicitly initialized to avoid erratic
//!                  hashing behavior.
//!     :param size: The size of the data to be hashed, in bytes
//!     :returns: The 64-bit alternative Fowler-Noll-Vo hash of the given :c:texpr:`data`
uint_fast64_t scc_hash_fnv1a_64(void const *data, size_t size);

//! .. _scc_hash_fnv1a:
//! .. c:function:: scc_hash_type scc_hash_fnv1a(void const *data, size_t size)
//!
//!     Simple `alternative Fowler-Noll-Vo hash
//!     <https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function#FNV-1a_hash>`_
//!     implementation.
//!
//!     :param data: Address of the data to be hashed. The data
//!                  is treated as a consecutive array of bytes.
//!                  Potential padding in structs must therefore
//!                  be explicitly initialized to avoid erratic
//!                  hashing behavior.
//!     :param size: The size of the data to be hashed, in bytes
//!     :returns: The alternative Fowler-Noll-Vo hash of the given :c:texpr:`data`
inline scc_hash_type scc_hash_fnv1a(void const *data, size_t size) {
#ifdef SCC_BITARCH_32
    return scc_hash_fnv1a_32(data, size);
#else
    return scc_hash_fnv1a_64(data, size);
#endif
}


#endif /* SCC_HASH_H */
