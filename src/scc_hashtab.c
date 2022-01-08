#include <scc/scc_hashtab.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SCC_HASHTAB_OCCUPIED ((scc_hashtab_metatype)0x8000u)
#define SCC_HASHTAB_GUARD    ((scc_hashtab_metatype)0x4000u)

enum { SCC_HASHTAB_HASHSHIFT = 50 };

#ifndef SCC_SIMD_ISA_UNSUPPORTED
extern long long scc_hashtab_impl_insert_probe(
    struct scc_hashtab *base,
    void *table,
    unsigned long long elemsize,
    unsigned long long hash
);
extern void scc_hashtab_impl_prep_iter(scc_hashtab_metatype *md, unsigned long long size);
extern long long scc_hashtab_impl_find_probe(
    struct scc_hashtab const *base,
    void const *table,
    unsigned long long elemsize,
    unsigned long long hash
);
#endif

size_t scc_hashtab_impl_bkoff(void const *tab);
size_t scc_hashtab_capacity(void const *tab);
size_t scc_hashtab_size(void const *tab);

static size_t scc_hashtab_md_size(struct scc_hashtab const *base);
static void scc_hashtab_init_mdguard(struct scc_hashtab *base);
static scc_hashtab_metatype *scc_hashtab_md(struct scc_hashtab *base);
static bool scc_hashtab_should_rehash(struct scc_hashtab const *base);
static struct scc_hashtab *scc_hashtab_resize(
    void *restrict *newtab,
    void *tab,
    struct scc_hashtab const *base,
    size_t elemsize
);
static bool scc_hashtab_emplace(void *tab, struct scc_hashtab *base, size_t elemsize);
static bool scc_hashtab_rehash(void **tab, struct scc_hashtab *base, size_t elemsize);

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

static inline bool scc_hashtab_should_rehash(struct scc_hashtab const *base) {
    /* TODO: this can likely be relaxed */
    return (base->ht_capacity >> 1u) < base->ht_size;
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

/* Hash, check for duplicates and, if none are found, emplace in table */
static bool scc_hashtab_emplace(void *tab, struct scc_hashtab *base, size_t elemsize) {
    unsigned long long hash = base->ht_hash(tab, elemsize);
    long long const index = scc_hashtab_impl_insert_probe(base, tab, elemsize, hash);
    if(index == -1) {
        return false;
    }
    /* tab holds address of base->ht_data[-1] */
    memcpy((unsigned char *)tab + (index + 1) * elemsize, tab, elemsize);
    /* Mark slot as occupied */
    scc_hashtab_md(base)[index] =
        (scc_hashtab_metatype)((hash >> SCC_HASHTAB_HASHSHIFT) | SCC_HASHTAB_OCCUPIED);

    return true;
}

/* Allocate enlarged hash table, initialize all but the data and metadata */
static struct scc_hashtab *scc_hashtab_resize(
    void *restrict *newtab,
    void *tab,
    struct scc_hashtab const *base,
    size_t elemsize
) {
    size_t const newcap = base->ht_capacity << 1u;
    /* Size of the struct up to and including ht_tmp */
    size_t const hdrsize = (unsigned char *)tab - (unsigned char const *)base + elemsize;
    /* Size of ht_data */
    size_t const datasize = newcap * elemsize;

    size_t const align = scc_alignof(scc_hashtab_metatype);
    /* Initial metadata offset */
    size_t mdoff = hdrsize + datasize;

    /* Ensure metadata array is properly aligned */
    mdoff = (mdoff + align - 1) & ~(align - 1);
    assert((mdoff & ~(align - 1)) == mdoff);

    size_t const size = mdoff + (newcap + scc_hashtab_impl_guardsz()) * sizeof(scc_hashtab_metatype);

    struct scc_hashtab *newbase = calloc(size, sizeof(unsigned char));
    if(!newbase) {
        return 0;
    }

    newbase->ht_eq = base->ht_eq;
    newbase->ht_hash = base->ht_hash;
    newbase->ht_mdoff = mdoff;
    newbase->ht_size = base->ht_size;
    newbase->ht_capacity = newcap;
    newbase->ht_dynalloc = 1;
    newbase->ht_fwoff = base->ht_fwoff;
    scc_hashtab_init_mdguard(newbase);

    *newtab = (unsigned char *)newbase + hdrsize - elemsize;
    ((unsigned char *)(*newtab))[-1] = base->ht_fwoff;
    return newbase;
}

/* Allocate a new table and insert all elements in current table. On success, replace
 * *tab with the new table */
static bool scc_hashtab_rehash(void **tab, struct scc_hashtab *base, size_t elemsize) {
    void *newtab;
    struct scc_hashtab *newbase = scc_hashtab_resize(&newtab, *tab, base, elemsize);
    if(!newbase) {
        return false;
    }

    scc_hashtab_metatype *md = scc_hashtab_md(base);
    /* Set occupied metadata entries to 1, non-occupied to 0 */
    scc_hashtab_impl_prep_iter(md, base->ht_capacity);

    /* Insert each element in old table */
    for(size_t i = 0u; i < base->ht_capacity; ++i) {
        if(md[i]) {
            memcpy(newtab, (unsigned char *)*(void **)tab + (i + 1) * elemsize, elemsize);
            scc_hashtab_emplace(newtab, newbase, elemsize);
        }
    }

    /* Copy ht_tmp of old table */
    memcpy(newtab, *tab, elemsize);
    scc_hashtab_free(*tab);
    *tab = newtab;
    return true;
}

bool scc_hashtab_impl_insert(void *tab, size_t elemsize) {
    struct scc_hashtab *base = scc_hashtab_impl_base(*(void **)tab);
    if(scc_hashtab_should_rehash(base)) {
        if(!scc_hashtab_rehash(tab, base, elemsize)) {
            return false;
        }
        /* *tab has been reallocated */
        base = scc_hashtab_impl_base(*(void **)tab);
    }
    if(scc_hashtab_emplace(*(void **)tab, base, elemsize)) {
        ++base->ht_size;
        return true;
    }
    return false;
}

void const *scc_hashtab_impl_find(void const *tab, size_t elemsize) {
    struct scc_hashtab const *base = scc_hashtab_impl_base_qual(tab, const);
    unsigned long long hash = base->ht_hash(tab, elemsize);
    long long const index = scc_hashtab_impl_find_probe(base, tab, elemsize, hash);
    if(index == -1) {
        return 0;
    }

    /* tab holds address of base->ht_data[-1] */
    return (void const *)((unsigned char const *)tab + (index + 1) * elemsize);
}

