#ifndef HASH_H
#define HASH_H

#include <stddef.h>

//! .. _scc_hash_fnv1a:
//! .. c:function:: unsigned long long scc_hash_fnv1a(void const *data, size_t size)
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
unsigned long long scc_hash_fnv1a(void const *data, size_t size);


#endif /* HASH_H */
