#include <scc/scc_hashtab.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SCC_HASHTAB_GUARD ((scc_hashtab_metatype)0x4000u)

#ifndef SCC_SIMD_ISA_UNSUPPORTED
long long scc_hashtab_impl_insert_probe(
    struct scc_hashtab *base,
    void *table,
    size_t elemsize,
    unsigned long long hash
);
#endif

size_t scc_hashtab_impl_bkoff(void const *tab);
size_t scc_hashtab_capacity(void const *tab);

static size_t scc_hashtab_md_size(struct scc_hashtab const *base);
static void scc_hashtab_init_mdguard(struct scc_hashtab *base);
static scc_hashtab_metatype *scc_hashtab_md(struct scc_hashtab *base);

static inline size_t scc_hashtab_md_size(struct scc_hashtab const *base) {
    return base->ht_capacity * sizeof(scc_hashtab_metatype);
}

static inline void scc_hashtab_init_mdguard(struct scc_hashtab *base) {
    scc_hashtab_metatype *guard =
        (void *)((unsigned char *)base + base->ht_mdoff + scc_hashtab_md_size(base));
    for(unsigned i = 0u; i < scc_hashtab_impl_guardsz(); ++i) {
        guard[i] = SCC_HASHTAB_GUARD;
    }
}

static inline scc_hashtab_metatype *scc_hashtab_md(struct scc_hashtab *base) {
    return (void *)((unsigned char *)base + base->ht_mdoff);
}

void *scc_hashtab_impl_init(void *inittab, scc_eq eq, scc_hash hash, size_t dataoff, size_t mdoff, size_t capacity) {
    struct scc_hashtab *tab = inittab;
    tab->ht_eq = eq;
    tab->ht_hash = hash;
    tab->ht_mdoff = mdoff;
    tab->ht_capacity = capacity;
    scc_hashtab_init_mdguard(tab);

    size_t const off = dataoff - offsetof(struct scc_hashtab, ht_fwoff) - sizeof(tab->ht_fwoff);
    assert(off < UCHAR_MAX);
    /* Power of 2 required */
    assert((capacity & ~(capacity - 1)) == capacity);

    /* To avoid future mishaps, would be triggered only with broken compilers atm */
    assert(!tab->ht_dynalloc);
    assert(!tab->ht_size);
    assert(!*((unsigned char *)tab + tab->ht_mdoff));

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

bool scc_hashtab_impl_insert(void *tab, size_t elemsize) {

#warning TODO: rehash

    struct scc_hashtab *base = scc_hashtab_impl_base(*(void **)tab);
    unsigned long long hash = base->ht_hash(*(void **)tab, elemsize);
    long long const index = scc_hashtab_impl_insert_probe(base, *(void **)tab, elemsize, hash);
    if(index == -1) {
        return false;
    }
    /* tab holds address of base->ht_data[-1] */
    memcpy((unsigned char *)*(void **)tab + (index + 1) * elemsize, *(void **)tab, elemsize);
    /* Mark slot as occupied */
    scc_hashtab_md(base)[index] = (scc_hashtab_metatype)((hash >> 50) | 0x8000u);

    ++base->ht_size;

    return true;
}

