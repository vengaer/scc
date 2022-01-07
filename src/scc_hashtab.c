#include <scc/scc_hashtab.h>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

size_t scc_hashtab_impl_bkoff(void const *tab);

void *scc_hashtab_impl_init(void *inittab, scc_eq eq, scc_hash hash, size_t dataoff, size_t mdoff, size_t capacity) {
    struct scc_hashtab *tab = inittab;
    tab->ht_eq = eq;
    tab->ht_hash = hash;
    tab->ht_mdoff = mdoff;
    tab->ht_size = 0u;
    tab->ht_capacity = capacity;
    tab->ht_dynalloc = 0;

    size_t const off = dataoff - offsetof(struct scc_hashtab, ht_fwoff) - sizeof(tab->ht_fwoff);
    assert(off < UCHAR_MAX);
    /* Power of 2 required */
    assert((capacity & ~(capacity - 1)) == capacity);
    tab->ht_fwoff = (unsigned char)off;

    unsigned char *ht_tmp = (unsigned char *)inittab + dataoff;
    ht_tmp[-1] = (unsigned char)off;
    return ht_tmp;
}

void scc_hashtab_free(void *tab) {
    struct scc_hashtab *base = scc_hashtab_impl_base(tab);
    if(base->ht_dynalloc) {
        free(base);
    }
}

unsigned long long scc_hashtab_fnv1a(void const *input, size_t size) {
#define SCC_FNV_OFFSET_BASIS 0xcbf29ce484222325ull
#define SCC_FNV_PRIME 0x100000001b3ull

    unsigned long long hash = SCC_FNV_OFFSET_BASIS;
    unsigned char const *data = input;
    for(size_t i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= SCC_FNV_PRIME;
    }
    return hash;

#undef SCC_FNV_OFFSET_BASIS
#undef SCC_FNV_PRIME
}

