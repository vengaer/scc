/*
 * Copyright (c) 2023 Vilhelm Engström
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCC_HASH_H
#define SCC_HASH_H

#include <stddef.h>
#include <stdint.h>

#include <scc/config.h>
#include <scc/murmur32.h>
#include <scc/murmur64.h>

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

uint_fast32_t scc_hash_fnv1a_32(void const *data, size_t size);

uint_fast64_t scc_hash_fnv1a_64(void const *data, size_t size);

inline scc_hash_type scc_hash_fnv1a(void const *data, size_t size) {
#ifdef SCC_BITARCH_32
    return scc_hash_fnv1a_32(data, size);
#else
    return scc_hash_fnv1a_64(data, size);
#endif
}


#if defined SCC_HAVE_UINT32_T || defined SCC_HAVE_UINT64_t
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
