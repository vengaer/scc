#include <scc/arch.h>
#include <scc/hashmap.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_HASHMAP_OCCUPIED = 0x80 };
enum { SCC_HASHMAP_VACATED = 0x7f };
enum { SCC_HASHMAP_HASHSHIFT = 57 };

size_t scc_hashmap_impl_bkpad(void const *map);
size_t scc_hashmap_capacity(void const *map);
size_t scc_hashmap_size(void const *map);

static inline void scc_hashmap_set_mdent(
    scc_hashmap_metatype *md,
    size_t index,
    scc_hashmap_metatype val,
    size_t capacity
) {
    md[index] = val;
    if (index < SCC_HASHMAP_GUARDSZ) {
        md[index + capacity] = val;
    }
}

static inline unsigned char scc_hashmap_calcpad(size_t coff) {
    size_t const fwoff = coff -
        offsetof(struct scc_hashmap_base, hm_fwoff) -
        sizeof(((struct scc_hashmap_base *)0)->hm_fwoff);
    assert(fwoff <= UCHAR_MAX);
    return fwoff;
}

static inline void scc_hashmap_set_bkoff(void *map, unsigned char bkoff) {
    ((unsigned char *)map)[-1] = bkoff;
}

static inline bool scc_hashmap_should_rehash(struct scc_hashmap_base const *base) {
    /* Rehash at 87.5% */
    return base->hm_size > (base->hm_capacity >> 1u) +
                           (base->hm_capacity >> 2u) +
                           (base->hm_capacity >> 3u);
}

static inline size_t scc_hashmap_sizeup(struct scc_hashmap_base const *base) {
    return base->hm_capacity << 1u;
}

static inline void *scc_hashmap_vals(struct scc_hashmap_base *base) {
    return (unsigned char *)base + base->hm_valoff;
}

static inline scc_hashmap_metatype *scc_hashmap_metadata(struct scc_hashmap_base *base) {
    return (void *)((unsigned char *)base + base->hm_mdoff);
}

bool scc_hashmap_emplace(void *map, struct scc_hashmap_base *base, size_t keysize, size_t valsize) {
    scc_hash_type hash = base->hm_hash(map, keysize);
    unsigned long long index = scc_hashmap_impl_probe_insert(base, map, keysize, hash);
    bool duplicate = index & SCC_HASHMAP_DUPLICATE;
    index &= ~SCC_HASHMAP_DUPLICATE;

    assert(index < base->hm_capacity);

    /* Insert key */
    void *dst = (unsigned char *)map + base->hm_pairsize + index * keysize;
    memcpy(dst, map, keysize);

    /* Insert value */
    dst = (unsigned char *)scc_hashmap_vals(base) + index * valsize;
    void const *src = (unsigned char *)map + keysize + base->hm_valpad;
    memcpy(dst, src, valsize);

    scc_hashmap_metatype ent = (scc_hashmap_metatype)(SCC_HASHMAP_OCCUPIED | (hash >> SCC_HASHMAP_HASHSHIFT));
    scc_hashmap_metatype *md = scc_hashmap_metadata(base);
    scc_hashmap_set_mdent(md, index, ent, base->hm_capacity);
    return duplicate;
}

static struct scc_hashmap_base *scc_hashmap_realloc(
    void *restrict *newmap,
    void const *map,
    struct scc_hashmap_base const *base,
    size_t keysize,
    size_t valsize,
    size_t cap
) {
    assert(scc_bits_is_power_of_2(cap));

    /* Size of map up to and including hm_curr */
    size_t const hdrsize =
        (unsigned char const *)map - (unsigned char const *)base + base->hm_pairsize;

    /* Offset of key array, no padding possible between hm_curr
     * and key array (although struct may contain trailing bytes) */
    size_t keyoff = scc_align(hdrsize, base->hm_keyalign);
    assert((keyoff & ~(base->hm_keyalign - 1u)) == keyoff);

    /* Offset of value array */
    size_t valoff = keyoff + cap * keysize;

    /* HACK: kill nonsensical mull mutation causing
     * underflow */
    assert(valoff != keyoff - cap * keysize);

    valoff = scc_align(valoff, base->hm_valalign);
    assert((valoff & ~(base->hm_valalign - 1u)) == valoff);

    /* Offset of metadata array */
    size_t mdoff = valoff + cap * valsize;
    mdoff = scc_align(mdoff, scc_alignof(scc_hashmap_metatype));
    assert((mdoff & ~(scc_alignof(scc_hashmap_metatype) - 1u)) == mdoff);

    scc_static_assert(sizeof(scc_hashmap_metatype) == 1u);
    size_t size = mdoff + cap + SCC_HASHMAP_GUARDSZ;
    scc_when_mutating(assert(size > mdoff + cap));
#ifdef SCC_CANARY_ENABLED
    size += SCC_HASHMAP_CANARYSZ;
#endif

    /* Allocate new map, ignore clang tidy being scared by scc_hashmap_base being
     * larger than unsigned char */
    struct scc_hashmap_base *newbase = calloc(size, sizeof(unsigned char)); /* NOLINT */
    if (!newbase) {
        return 0;
    }

    newbase->hm_eq = base->hm_eq;
    newbase->hm_hash = base->hm_hash;
    newbase->hm_valoff = valoff;
    newbase->hm_mdoff = mdoff;
    newbase->hm_size = base->hm_size;
    newbase->hm_capacity = cap;
    newbase->hm_pairsize = base->hm_pairsize;
    newbase->hm_keyalign = base->hm_keyalign;
    newbase->hm_valalign = base->hm_valalign;
    newbase->hm_dynalloc = 1;
    newbase->hm_valpad = base->hm_valpad;
    newbase->hm_fwoff = base->hm_fwoff;
    SCC_ON_PERFTRACK(newbase->hm_perf = base->hm_perf);
    SCC_ON_PERFTRACK(newbase->hm_perf.ev_bytesz = size);

    *newmap = (unsigned char *)newbase + hdrsize - base->hm_pairsize;
    scc_hashmap_set_bkoff(*newmap, base->hm_fwoff);
    return newbase;
}

static bool scc_hashmap_rehash(
    void **map,
    struct scc_hashmap_base *base,
    size_t keysize,
    size_t valsize,
    size_t cap
) {
    void *newmap;
    struct scc_hashmap_base *newbase = scc_hashmap_realloc(&newmap, *map, base, keysize, valsize, cap);
    if (!newbase) {
        return false;
    }

    scc_hashmap_metatype *md = scc_hashmap_metadata(base);

    unsigned char *keybase = (unsigned char *)*(void **)map + base->hm_pairsize;
    unsigned char *valbase = scc_hashmap_vals(base);

    for (size_t i = 0u; base->hm_size; ++i) {
        assert(i < base->hm_capacity);
        if (md[i] & SCC_HASHMAP_OCCUPIED) {
            /* Copy key */
            memcpy(newmap, keybase + i * keysize, keysize);
            /* Copy value */
            memcpy((unsigned char *)newmap + keysize + base->hm_valpad, valbase + i * valsize, valsize);
            (void)scc_hashmap_emplace(newmap, newbase, keysize, valsize);
            --base->hm_size;
        }
    }

    SCC_ON_PERFTRACK(++newbase->hm_perf.ev_n_rehashes);

    /* Copy hm_curr of old map */
    memcpy(newmap, *map, base->hm_pairsize);
    scc_hashmap_free(*map);
    *map = newmap;
    return true;
}

void *scc_hashmap_impl_new(struct scc_hashmap_base *base, size_t coff, size_t valoff, size_t keysize) {
    scc_static_assert(sizeof(scc_hashmap_metatype) == 1u);
    scc_canary_init((unsigned char *)base + base->hm_mdoff + base->hm_capacity + SCC_HASHMAP_GUARDSZ, SCC_HASHMAP_CANARYSZ);

    size_t const valpad = valoff - keysize;
    assert(valpad <= UCHAR_MAX);
    base->hm_valpad = valpad;
    base->hm_fwoff = scc_hashmap_calcpad(coff);
    unsigned char *map = (unsigned char *)base + coff;
    scc_hashmap_set_bkoff(map, base->hm_fwoff);
    return map;
}

void *scc_hashmap_impl_new_dyn(struct scc_hashmap_base const *sbase, size_t mapsize, size_t coff, size_t valoff, size_t keysize) {
    struct scc_hashmap_base *base = calloc(mapsize, sizeof(unsigned char));
    if (!base) {
        return 0;
    }

    scc_memcpy(base, sbase, sizeof(*sbase));

    void *map = scc_hashmap_impl_new(base, coff, valoff, keysize);
    base->hm_dynalloc = 1;
    return map;
}

void scc_hashmap_free(void *map) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    if (base->hm_dynalloc) {
        free(base);
    }
}

bool scc_hashmap_impl_insert(void *mapaddr, size_t keysize, size_t valsize) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(*(void **)mapaddr);
    if (scc_hashmap_should_rehash(base)) {
        size_t const newcap = scc_hashmap_sizeup(base);
        if (!scc_hashmap_rehash(mapaddr, base, keysize, valsize, newcap)) {
            return false;
        }

        /* Map has been reallocated */
        base = scc_hashmap_impl_base(*(void **)mapaddr);
    }
    if (!scc_hashmap_emplace(*(void **)mapaddr, base, keysize, valsize)) {
        ++base->hm_size;
    }
    return true;
}

void *scc_hashmap_impl_find(void *map, size_t keysize, size_t valsize) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    if (!base->hm_size) {
        return 0;
    }

    scc_hash_type hash = base->hm_hash(map, keysize);
    long long const index = scc_hashmap_impl_probe_find(base, map, keysize, hash);
    if (index == -1ll) {
        return 0;
    }
    assert(base->hm_size);
    assert(index >= 0ll && (size_t)index < base->hm_capacity);

    unsigned char *valbase = scc_hashmap_vals(base);
    return (void *)(valbase + index * valsize);
}

bool scc_hashmap_impl_remove(void *map, size_t keysize) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    if (!base->hm_size) {
        return false;
    }

    scc_hash_type const hash = base->hm_hash(map, keysize);
    long long const index = scc_hashmap_impl_probe_find(base, map, keysize, hash);
    if (index == -1ll) {
        return false;
    }

    assert(base->hm_size);
    assert(index >= 0ll && (size_t)index < base->hm_capacity);

    scc_hashmap_metatype *md = scc_hashmap_metadata(base);
    scc_hashmap_set_mdent(md, index, SCC_HASHMAP_VACATED, base->hm_capacity);
    --base->hm_size;
    return true;
}

void scc_hashmap_clear(void *map) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    scc_hashmap_metatype *md = scc_hashmap_metadata(base);
    scc_static_assert(sizeof(*md) == 1u);
    memset(md, 0, (base->hm_capacity + SCC_HASHMAP_GUARDSZ));
    base->hm_size = 0u;
}

void *scc_hashmap_clone(void const *map) {
    struct scc_hashmap_base const *obase = scc_hashmap_impl_base_qual(map, const);
    scc_static_assert(sizeof(scc_hashmap_metatype) == 1);
    size_t sz = obase->hm_mdoff + obase->hm_capacity + SCC_HASHMAP_GUARDSZ;
    scc_when_mutating(assert(sz > obase->hm_mdoff + obase->hm_capacity));
#ifdef SCC_CANARY_ENABLED
    sz += SCC_HASHMAP_CANARYSZ;
#endif
    struct scc_hashmap_base *nbase = malloc(sz);
    if (!nbase) {
        return 0;
    }
    scc_memcpy(nbase, obase, sz);
    nbase->hm_dynalloc = 1;
    return (unsigned char *)nbase + offsetof(struct scc_hashmap_base, hm_fwoff) + nbase->hm_fwoff + sizeof(nbase->hm_fwoff);
}
