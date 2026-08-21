#include <scc/arch.h>
#include <scc/bug.h>
#include <scc/hashtab.h>
#include <scc/mem.h>
#include <scc/perf.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_HASHTAB_OCCUPIED = 0x80 };
enum { SCC_HASHTAB_VACATED = 0x7f };
enum { SCC_HASHTAB_HASHSHIFT = 57 };

size_t scc_hashtab_capacity(void const *tab);
size_t scc_hashtab_size(void const *tab);
size_t scc_hashtab_impl_bkpad(void const *tab);

static inline void scc_hashtab_set_mdent(scc_hashtab_metatype *md, size_t index, scc_hashtab_metatype val, size_t capacity) {
    md[index] = val;
    if (index < SCC_HASHTAB_GUARDSZ) {
        md[index + capacity] = val;
    }
}

static inline unsigned char scc_hashtab_calcpad(size_t coff) {
    size_t const fwoff = coff -
        offsetof(struct scc_hashtab_base, ht_fwoff) -
        sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
    assert(fwoff <= UCHAR_MAX);
    return fwoff;
}

static inline void scc_hashtab_set_bkoff(void *tab, unsigned char bkoff) {
    ((unsigned char *)tab)[-1] = bkoff;
}

static inline bool scc_hashtab_should_rehash(struct scc_hashtab_base const *base) {
    /* Rehash at 87.5% */
    return base->ht_size > (base->ht_capacity >> 1u) +
                           (base->ht_capacity >> 2u) +
                           (base->ht_capacity >> 3u);
}

static inline size_t scc_hashtab_sizeup(struct scc_hashtab_base const *base) {
    return base->ht_capacity << 1u;
}

static inline scc_hashtab_metatype *scc_hashtab_metadata(struct scc_hashtab_base *base) {
    return (void *)((unsigned char *)base + base->ht_mdoff);
}

static bool scc_hashtab_emplace(void *tab, struct scc_hashtab_base *base, size_t elemsize) {
    scc_hash_type const hash = base->ht_hash(tab, elemsize);
    long long index = scc_hashtab_impl_probe_insert(base, tab, elemsize, hash);

    SCC_ON_PERFTRACK(++base->ht_perf.ev_n_hash);

    if (index == -1ll) {
        return false;
    }

    assert(index >= 0ll && (size_t)index < base->ht_capacity);
    /* tab holds address of base->ht_data[-1] */
    memcpy((unsigned char *)tab + (index + 1ll) * elemsize, tab, elemsize);

    scc_hashtab_metatype *md = scc_hashtab_metadata(base);
    /* Mark slot as occupied */
    scc_hashtab_metatype ent = (scc_hashtab_metatype)(SCC_HASHTAB_OCCUPIED | (hash >> SCC_HASHTAB_HASHSHIFT));
    scc_hashtab_set_mdent(md, index, ent, base->ht_capacity);
    return true;
}

static struct scc_hashtab_base *scc_hashtab_realloc(
    void *restrict *newtab,
    void const *tab,
    struct scc_hashtab_base const *base,
    size_t elemsize,
    size_t cap
) {
    assert(scc_bits_is_power_of_2(cap));

    /* Size of table up to and including ht_curr */
    size_t const hdrsize =
        (unsigned char const *)tab - (unsigned char const *)base + elemsize;

    /* Size of ht_data for new table */
    size_t const datasize = cap * elemsize;
    size_t const align = scc_alignof(scc_hashtab_metatype);

    /* Initial metadata offset, no padding allowed between
     * ht_curr and ht_data */
    size_t mdoff = hdrsize + datasize;

    /* Align metadata */
    mdoff = (mdoff + align - 1) & ~(align - 1);
    assert((mdoff & ~(align - 1)) == mdoff);


    scc_static_assert(sizeof(scc_hashtab_metatype) == 1u);
    size_t size = mdoff + cap + SCC_HASHTAB_GUARDSZ;
    scc_when_mutating(assert(size > mdoff + cap));
#ifdef SCC_CANARY_ENABLED
    size += SCC_HASHTAB_CANARYSZ;
#endif

    /* Allocate new hash table
     * Ignore clang tidy complaining about struct scc_hashtab_base being
     * larger than unsigned char */
    struct scc_hashtab_base *newbase = calloc(size, sizeof(unsigned char)); /* NOLINT */
    if (!newbase) {
        return 0;
    }

    newbase->ht_eq = base->ht_eq;
    newbase->ht_hash = base->ht_hash;
    newbase->ht_mdoff = mdoff;
    newbase->ht_size = base->ht_size;
    newbase->ht_capacity = cap;
    newbase->ht_dynalloc = 1;
    newbase->ht_fwoff = base->ht_fwoff;

    SCC_ON_PERFTRACK(newbase->ht_perf = base->ht_perf);
    SCC_ON_PERFTRACK(newbase->ht_perf.ev_bytesz = size);

    *newtab = (unsigned char *)newbase + hdrsize - elemsize;
    scc_hashtab_set_bkoff(*newtab, base->ht_fwoff);
    return newbase;
}

static bool scc_hashtab_rehash(void **tab, struct scc_hashtab_base *base, size_t elemsize, size_t cap) {
    void *newtab;
    struct scc_hashtab_base *newbase = scc_hashtab_realloc(&newtab, *tab, base, elemsize, cap);
    if (!newbase) {
        return false;
    }

    scc_hashtab_metatype *md = scc_hashtab_metadata(base);

    for (size_t i = 0u; base->ht_size; ++i) {
        assert(i < base->ht_capacity);
        if (md[i] & SCC_HASHTAB_OCCUPIED) {
            memcpy(newtab, (unsigned char *)*(void **)tab + (i + 1u) * elemsize, elemsize);
            scc_bug_on(!scc_hashtab_emplace(newtab, newbase, elemsize));
            --base->ht_size;
        }
    }

    SCC_ON_PERFTRACK(++newbase->ht_perf.ev_n_rehashes);

    /* Copy ht_curr of old table */
    memcpy(newtab, *tab, elemsize);
    scc_hashtab_free(*tab);
    *tab = newtab;
    return true;
}

static inline void const *scc_hashtab_impl_iter_next_occupied(
    struct scc_hashtab_base *base,
    void *tab,
    size_t elemsize,
    size_t start
) {
    scc_hashtab_metatype *md = scc_hashtab_metadata((struct scc_hashtab_base *)base);

    for (size_t i = start; i < base->ht_capacity; ++i) {
        if (md[i] & SCC_HASHTAB_OCCUPIED)
            return (unsigned char const *)tab + (i + (size_t)1) * elemsize;
    }

    return 0;
}

bool scc_hashtab_impl_insert(void *tabaddr, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(*(void **)tabaddr);
    if (scc_hashtab_should_rehash(base)) {
        size_t const newcap = scc_hashtab_sizeup(base);
        if (!scc_hashtab_rehash(tabaddr, base, elemsize, newcap)) {
            return false;
        }

        /* Table has been reallocated */
        base = scc_hashtab_impl_base(*(void **)tabaddr);
    }
    if (!scc_hashtab_emplace(*(void **)tabaddr, base, elemsize)) {
        return false;
    }

    SCC_ON_PERFTRACK(++base->ht_perf.ev_n_inserts);
    ++base->ht_size;
    return true;
}

void const *scc_hashtab_impl_find(void const *tab, size_t elemsize) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    if (!base->ht_size) {
        return 0;
    }
    scc_hash_type const hash = base->ht_hash(tab, elemsize);
    long long const index = scc_hashtab_impl_probe_find(base, tab, elemsize, hash);
    if (index == -1ll) {
        return 0;
    }
    assert(base->ht_size);
    assert(index >= 0ll && (size_t)index < base->ht_capacity);

    /* tab holds &base->ht_data[-1] */
    return (void const *)((unsigned char const *)tab + (index + 1ull) * elemsize);
}

void *scc_hashtab_impl_new(struct scc_hashtab_base *base, size_t coff, size_t mdoff) {
    base->ht_mdoff = mdoff;
    base->ht_fwoff = scc_hashtab_calcpad(coff);
    unsigned char *tab = (unsigned char *)base + coff;

    scc_static_assert(sizeof(scc_hashtab_metatype) == 1u);

    scc_canary_init((unsigned char *)base + mdoff + base->ht_capacity + SCC_HASHTAB_GUARDSZ, SCC_HASHTAB_CANARYSZ);
    SCC_ON_PERFTRACK(base->ht_perf.ev_bytesz = mdoff + SCC_HASHTAB_STACKCAP + SCC_HASHTAB_GUARDSZ);

    scc_hashtab_set_bkoff(tab, base->ht_fwoff);
    return tab;
}

void *scc_hashtab_impl_new_dyn(scc_hashtab_eq eq, scc_hashtab_hash hash, size_t cap, size_t tabsz, size_t coff, size_t mdoff) {
    struct scc_hashtab_base *base = calloc(tabsz, sizeof(unsigned char));
    if (!base) {
        return 0;
    }

    base->ht_eq = eq;
    base->ht_hash = hash;
    base->ht_capacity = cap;
    void *tab = scc_hashtab_impl_new(base, coff, mdoff);
    base->ht_dynalloc = 1;
    return tab;
}

void scc_hashtab_free(void *tab) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);
    if (base->ht_dynalloc) {
        free(base);
    }
}

bool scc_hashtab_impl_reserve(void *tabaddr, size_t capacity, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(*(void **)tabaddr);
    if (capacity <= base->ht_capacity) {
        return true;
    }
    if (!scc_bits_is_power_of_2(capacity)) {
        unsigned n;
        for (n = 0u; capacity; capacity >>= 1u, ++n);
        capacity = 1u << n;
    }
    assert(scc_bits_is_power_of_2(capacity));
    if (!scc_hashtab_rehash(tabaddr, base, elemsize, capacity)) {
        return false;
    }

    return true;
}

bool scc_hashtab_impl_remove(void *tab, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);
    if (!base->ht_size) {
        return false;
    }

    scc_hash_type const hash = base->ht_hash(tab, elemsize);

    long long const index = scc_hashtab_impl_probe_find(base, tab, elemsize, hash);
    if (index == -1ll) {
        return false;
    }

    assert(base->ht_size);
    assert(index >= 0ll && (size_t)index < base->ht_capacity);

    scc_hashtab_metatype *md = scc_hashtab_metadata(base);
    scc_hashtab_set_mdent(md, index, SCC_HASHTAB_VACATED, base->ht_capacity);
    --base->ht_size;
    return true;
}

void scc_hashtab_clear(void *tab) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);
    scc_hashtab_metatype *md = scc_hashtab_metadata(base);
    scc_static_assert(sizeof(*md) == 1u);
    memset(md, 0, base->ht_capacity + SCC_HASHTAB_GUARDSZ);
    base->ht_size = 0u;
}

void *scc_hashtab_clone(void const *tab) {
    struct scc_hashtab_base const *obase = scc_hashtab_impl_base_qual(tab, const);
    scc_static_assert(sizeof(scc_hashtab_metatype) == 1);
    size_t sz = obase->ht_mdoff + obase->ht_capacity + SCC_HASHTAB_GUARDSZ;
    scc_when_mutating(assert(sz > obase->ht_mdoff + obase->ht_capacity));
#ifdef SCC_CANARY_ENABLED
    sz += SCC_HASHTAB_CANARYSZ;
#endif
    struct scc_hashtab_base *nbase = malloc(sz);
    if (!nbase) {
        return 0;
    }
    scc_memcpy(nbase, obase, sz);
    nbase->ht_dynalloc = 1;
    return (unsigned char *)nbase + offsetof(struct scc_hashtab_base, ht_fwoff) + nbase->ht_fwoff + sizeof(nbase->ht_fwoff);
}

void const *scc_hashtab_impl_iter_begin(void *tab, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);
    return scc_hashtab_impl_iter_next_occupied(base, tab, elemsize, 0u);
}

void const *scc_hashtab_impl_iter_next(void *tab, size_t elemsize, void const *iter) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);

    ptrdiff_t pslot = (((unsigned char const *)iter) -
        ((unsigned char const *)tab + elemsize)) / elemsize;
    assert(pslot >= 0);
    return scc_hashtab_impl_iter_next_occupied(base, tab, elemsize, (unsigned)pslot + 1u);
}
