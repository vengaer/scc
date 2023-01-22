#include <scc/hashmap.h>
#include "asm/asm_generic.h"

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

//? .. c:function:: void scc_hashmap_set_mdent(\
//?        scc_hashmap_metatype *md, size_t index, \
//?        scc_hashmap_metatype val, size_t capacity)
//?
//?     Set metadata entry at given index, duplicating low
//?     entries in the guard.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param md: Address of the first element in the metadata array
//?     :param index: The index at which the entry is to be set
//?     :param val: Value to write to the given index
//?     :param capacity: Map capacity for calculating guard offset
static inline void scc_hashmap_set_mdent(
    scc_hashmap_metatype *md,
    size_t index,
    scc_hashmap_metatype val,
    size_t capacity
) {
    md[index] = val;
    if(index < SCC_HASHMAP_GUARDSZ) {
        md[index + capacity] = val;
    }
}

//? .. c:function:: unsigned char scc_hashmap_calcpad(size_t coff)
//?
//?     Calculate the number of padding bytes between
//?     :ref:`hm_fwoff <unsigned_char_hm_fwoff>` and
//?     :ref:`hm_curr <scc_hashmap_impl_pair_hm_curr>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param coff: Base-relative offset of :code:`hm_curr`
//?     :returns: Number of padding bytes between :code:`hm_fwoff`
//?               and :code:`hm_curr`.
static inline unsigned char scc_hashmap_calcpad(size_t coff) {
    size_t const fwoff = coff -
        offsetof(struct scc_hashmap_base, hm_fwoff) -
        sizeof(((struct scc_hashmap_base *)0)->hm_fwoff);
    assert(fwoff <= UCHAR_MAX);
    return fwoff;
}

//? .. c:function:: void scc_hashmap_set_bkoff(void *map, unsigned char bkoff)
//?
//?     Set the :ref:`hm_bkoff <unsigned_char_hm_bkoff>` field in the map.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: Hash map handle
//?     :param bkoff: Value to write
static inline void scc_hashmap_set_bkoff(void *map, unsigned char bkoff) {
    ((unsigned char *)map)[-1] = bkoff;
}

//? .. c:function:: _Bool scc_hashmap_should_rehash(struct scc_hashmap_base const *base)
//?
//?     Check whether the load factor is high enough to require a rehash
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of the hash map base
//?     :returns: :code:`true` if the map shoud be rehashed, otherwise :code:`false`.
static inline bool scc_hashmap_should_rehash(struct scc_hashmap_base const *base) {
    /* Rehash at 87.5% */
    return base->hm_size > (base->hm_capacity >> 1u) +
                           (base->hm_capacity >> 2u) +
                           (base->hm_capacity >> 3u);
}

//? .. c:function:: size_t scc_hashmap_sizeup(struct scc_hashmap_base const *base)
//?
//?     Compute capacity after resize for the given hash map
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of the base struct of the hash map
//?     :returns: Capacity the given map would have after the next rehash
static inline size_t scc_hashmap_sizeup(struct scc_hashmap_base const *base) {
    return base->hm_capacity << 1u;
}

//? .. c:function:: void *scc_hashmap_vals(struct scc_hashmap_base *base)
//?
//?     Obtain address of the value address in the map
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the map
//?     :returns: Address of the value array of the hash map
static inline void *scc_hashmap_vals(struct scc_hashmap_base *base) {
    return (unsigned char *)base + base->hm_valoff;
}

//? .. c:function:: scc_hashmap_metatype *scc_hashmap_metadata(struct scc_hashmap_base *base)
//?
//?     Obtain address of the metadata array in the map
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the map
//?     :returns: Address of the metadata array of the hash map
static inline scc_hashmap_metatype *scc_hashmap_metadata(struct scc_hashmap_base *base) {
    return (void *)((unsigned char *)base + base->hm_mdoff);
}

//? .. c:function:: _Bool scc_hashmap_emplace(void *map, struct scc_hashmap_base *base, size_t keysize, size_t valsize)
//?
//?     Emplace the given key-value pair in the map. Return :code:`true` if the
//?     key already existed in the map. The value is inserted regardless.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: Hash map handle
//?     :param base: Base address of the map
//?     :param keysize: Size of the key type
//?     :param valsize: Size of the value type
//?     :returns: :code:`true` if the key already existed in the map, otherwise :code:`false`
bool scc_hashmap_emplace(void *map, struct scc_hashmap_base *base, size_t keysize, size_t valsize) {
    unsigned long long hash = base->hm_hash(map, keysize);
    unsigned long long index = scc_hashmap_probe_insert(base, map, keysize, hash);
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

//? .. c:function:: struct scc_hashmap_base *scc_hashmap_realloc(\
//?        void *restrict *newmap, void const *map, \
//?        struct scc_hashmap_base const *base, size_t keysize, \
//?        size_t valsize, size_t cap)
//?
//?     Allocate a new hash map, fill in the fields of its base and return
//?     the address of the latter.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param newmap: Address of a new handle to be used for referring to
//?                    the soon-to-be newly allocated map
//?     :param map: Handle to the original map
//?     :param base: Base address of the original map
//?     :param keysize: Size of the key type
//?     :param valsize: Size of the value type
//?     :cap: Capacity of the new map
//?     :returns: Base address of a newly allocated hash map, or :code:`NULL`
//?               on allocation failure.
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
    size_t size = mdoff + (cap + SCC_HASHMAP_GUARDSZ);
#ifdef SCC_CANARY_ENABLED
    size += SCC_HASHMAP_CANARYSZ;
#endif

    /* Allocate new map, ignore clang tidy being scared by scc_hashmap_base being
     * larger than unsigned char */
    struct scc_hashmap_base *newbase = calloc(size, sizeof(unsigned char)); /* NOLINT */
    if(!newbase) {
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

//? .. c:function:: _Bool scc_hashmap_rehash(\
//?        void **map, struct scc_hashmap_base *base, \
//?        size_t keysize, size_t valsize, size_t cap)
//?
//?     Reallocate and rehash the hash map. On success, :c:texpr:`*map`
//?     is updated to refer to the new map and :code:`true` is
//?     returned. On failure, :c:texpr:`*map` is left unchanged.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param map: Address of the map handle
//?     :param base: Base address of the hash map
//?     :param keysize: Size of the key type
//?     :param valsize: Size of the value type
//?     :param cap: Capacity of the new map
//?     :returns: :code:`true` on successful rehashing, :code:`false` otherwise
static bool scc_hashmap_rehash(
    void **map,
    struct scc_hashmap_base *base,
    size_t keysize,
    size_t valsize,
    size_t cap
) {
    void *newmap;
    struct scc_hashmap_base *newbase = scc_hashmap_realloc(&newmap, *map, base, keysize, valsize, cap);
    if(!newbase) {
        return false;
    }

    scc_hashmap_metatype *md = scc_hashmap_metadata(base);

    unsigned char *keybase = (unsigned char *)*(void **)map + base->hm_pairsize;
    unsigned char *valbase = scc_hashmap_vals(base);

    for(size_t i = 0u; base->hm_size; ++i) {
        assert(i < base->hm_capacity);
        if(md[i] & SCC_HASHMAP_OCCUPIED) {
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

bool scc_hashmap_impl_insert(void *mapaddr, size_t keysize, size_t valsize) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(*(void **)mapaddr);
    if(scc_hashmap_should_rehash(base)) {
        size_t const newcap = scc_hashmap_sizeup(base);
        if(!scc_hashmap_rehash(mapaddr, base, keysize, valsize, newcap)) {
            return false;
        }

        /* Map has been reallocated */
        base = scc_hashmap_impl_base(*(void **)mapaddr);
    }
    if(!scc_hashmap_emplace(*(void **)mapaddr, base, keysize, valsize)) {
        ++base->hm_size;
    }
    return true;
}

void *scc_hashmap_impl_find(void *map, size_t keysize, size_t valsize) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    if(!base->hm_size) {
        return 0;
    }

    unsigned long long hash = base->hm_hash(map, keysize);
    long long const index = scc_hashmap_probe_find(base, map, keysize, hash);
    if(index == -1ll) {
        return 0;
    }
    assert(base->hm_size);
    assert(index >= 0ll && (size_t)index < base->hm_capacity);

    unsigned char *valbase = scc_hashmap_vals(base);
    return (void *)(valbase + index * valsize);
}

bool scc_hashmap_impl_remove(void *map, size_t keysize) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    if(!base->hm_size) {
        return false;
    }

    unsigned long long const hash = base->hm_hash(map, keysize);
    long long const index = scc_hashmap_probe_find(base, map, keysize, hash);
    if(index == -1ll) {
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
    /* Kill mutant */
    assert(sz > obase->hm_mdoff + obase->hm_capacity);
#ifdef SCC_CANARY_ENABLED
    sz += SCC_HASHMAP_CANARYSZ;
#endif
    struct scc_hashmap_base *nbase = malloc(sz);
    if(!nbase) {
        return 0;
    }
    scc_memcpy(nbase, obase, sz);
    nbase->hm_dynalloc = 1;
    return (unsigned char *)nbase + offsetof(struct scc_hashmap_base, hm_fwoff) + nbase->hm_fwoff + sizeof(nbase->hm_fwoff);
}
