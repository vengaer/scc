#include <scc/hash.h>

unsigned long long scc_hash_fnv1a(void const *data, size_t size) {
#define SCC_FNV_OFFSET_BASIS 0xcbf29ce484222325ull
#define SCC_FNV_PRIME 0x100000001b3ull

    unsigned long long hash = SCC_FNV_OFFSET_BASIS;
    unsigned char const *dptr = data;
    for(size_t i = 0; i < size; ++i) {
        hash ^= dptr[i];
        hash *= SCC_FNV_PRIME;
    }
    return hash;

#undef SCC_FNV_OFFSET_BASIS
#undef SCC_FNV_PRIME
}
