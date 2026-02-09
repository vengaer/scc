#include <scc/hash.h>

scc_hash_type scc_hash_fnv1a(void const *data, size_t size);
void scc_hash_murmur128(struct scc_digest128 *digest, void const *data,
        size_t size, uint_fast32_t seed);

uint_fast32_t scc_hash_fnv1a_32(void const *data, size_t size) {
#define SCC_FNV_OFFSET_BASIS UINT32_C(0x811c9dc5)
#define SCC_FNV_PRIME UINT32_C(0x01000193)
    uint_fast32_t hash = SCC_FNV_OFFSET_BASIS;
    unsigned char const *dptr = data;
    for (size_t i = 0; i < size; ++i) {
        hash ^= dptr[i];
        hash *= SCC_FNV_PRIME;
    }
    return hash;
#undef SCC_FNV_OFFSET_BASIS
#undef SCC_FNV_PRIME
}

uint_fast64_t scc_hash_fnv1a_64(void const *data, size_t size) {
#define SCC_FNV_OFFSET_BASIS UINT64_C(0xcbf29ce484222325)
#define SCC_FNV_PRIME UINT64_C(0x100000001b3)

    uint_fast64_t hash = SCC_FNV_OFFSET_BASIS;
    unsigned char const *dptr = data;
    for (size_t i = 0; i < size; ++i) {
        hash ^= dptr[i];
        hash *= SCC_FNV_PRIME;
    }
    return hash;

#undef SCC_FNV_OFFSET_BASIS
#undef SCC_FNV_PRIME
}
