#include <scc/scc_hashmap.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

size_t scc_hashmap_impl_bkpad(void const *map);
size_t scc_hashmap_capacity(void const *map);
size_t scc_hashmap_size(void const *map);

/* scc_hashmap_calcpad
 *
 * Calculate the number of padding bytes between hm_fwoff and hm_curr
 *
 * size_t coff
 *      Offset of hm_curr relative the base of the struct
 */
static inline unsigned char scc_hashmap_calcpad(size_t coff) {
    size_t const fwoff = coff -
        offsetof(struct scc_hashmap_base, hm_fwoff) -
        sizeof(((struct scc_hashmap_base *)0)->hm_fwoff);
    assert(fwoff <= UCHAR_MAX);
    return fwoff;
}

/* scc_hashmap_set_bkoff
 *
 * Set the hm_bkoff field
 *
 * void *map
 *      Handle referring to the hash map
 *
 * unsigned char bkoff
 *      The value to set
 */
static inline void scc_hashmap_set_bkoff(void *map, unsigned char bkoff) {
    ((unsigned char *)map)[-1] = bkoff;
}

void *scc_hashmap_impl_init(struct scc_hashmap_base *base, size_t coff, size_t valoff, size_t keysize) {
    size_t const valpad = valoff - keysize;
    assert(valpad <= UCHAR_MAX);
    base->hm_valpad = valpad;
    base->hm_fwoff = scc_hashmap_calcpad(coff);
    unsigned char *map = (unsigned char *)base + coff;
    scc_hashmap_set_bkoff(map, base->hm_fwoff);
    return map;
}

unsigned long long scc_hashmap_fnv1a(void const *data, size_t size) {
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

void scc_hashmap_free(void *map) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    if(base->hm_dynalloc) {
        free(base);
    }
}

