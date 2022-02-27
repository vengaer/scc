#include <scc/scc_hashtab.h>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

static inline size_t scc_hashtab_impl_bkpad(void const *handle);

static inline unsigned char scc_hashtab_calc_fwoff(size_t coff) {
    size_t const fwoff = coff -
        offsetof(struct scc_hashtab_base, ht_fwoff) -
        sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
    assert(fwoff <= UCHAR_MAX);
    return fwoff;
}

static inline void scc_hashtab_set_bkoff(void *handle, unsigned char bkoff) {
    ((unsigned char *)handle)[-1] = bkoff;
}

void *scc_hashtab_impl_init(struct scc_hashtab_base *base, scc_eq eq, scc_hash hash, size_t coff, size_t mdoff, size_t cap) {
    base->ht_eq = eq;
    base->ht_hash = hash;
    base->ht_mdoff = mdoff;
    base->ht_capacity = cap;
    base->ht_fwoff = scc_hashtab_calc_fwoff(coff);
    unsigned char *tab = (unsigned char *)base + coff;
    scc_hashtab_set_bkoff(tab, base->ht_fwoff);
    return tab;
}

unsigned long long scc_hashtab_fnv1a(void const *data, size_t size) {
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

void scc_hashtab_free(void *handle) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(handle);
    if(base->ht_dynalloc) {
        free(base);
    }
}
