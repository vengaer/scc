#include <scc/scc_hashmap.h>
#include <scc/scc_perf.h>

#include "asm/asm_generic.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_HASHMAP_OCCUPIED = 0x80 };
enum { SCC_HASHMAP_HASHSHIFT = 57 };

#define scc_hashmap_is_power_of_2(val) \
    (((val) & ~((val) - 1)) == (val))


size_t scc_hashmap_impl_bkpad(void const *map);
size_t scc_hashmap_capacity(void const *map);
size_t scc_hashmap_size(void const *map);

/* scc_hashmap_set_mdent
 *
 * Set metadata entry at given index, duplicating
 * low entries in the guard
 *
 * scc_hashmap_metatype *md
 *      Address of the first element in the metadata
 *      array
 *
 * size_t index
 *      The index in which the value is to be set
 *
 * scc_hashmap_metatype val
 *      The value to write to the given index
 *
 * size_t capacity
 *      Map capacity for calculating guard offset
 */
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

/* scc_hashmap_should_rehash
 *
 * Return true if the load factor is large enough to merit a rehash
 *
 * struct scc_hashmap_base const *base
 *      Address of the hash map base
 */
static inline bool scc_hashmap_should_rehash(struct scc_hashmap_base const *base) {
    /* Rehash at 87.5% */
    return base->hm_size > (base->hm_capacity >> 1u) +
                           (base->hm_capacity >> 2u) +
                           (base->hm_capacity >> 3u);
}

/* scc_hashmap_sizeup
 *
 * Compute capacity after resize for given hash map
 *
 * struct scc_hashmap_base const *base
 *      Address of the hash map base
 */
static inline size_t scc_hashmap_sizeup(struct scc_hashmap_base const *base) {
    return base->hm_capacity << 1u;
}

/* scc_hashmap_vals
 *
 * Return address of the value array for the given base
 *
 * struct scc_hashmap_base *base
 *      Base address of the map
 */
static inline void *scc_hashmap_vals(struct scc_hashmap_base *base) {
    return (unsigned char *)base + base->hm_valoff;
}

/* scc_hashmap_metadata
 *
 * Return address of first element of the metadata array in *base
 *
 * struct scc_hashmap_base *base
 *      Address of the hash map base
 */
static inline scc_hashmap_metatype *scc_hashmap_metadata(struct scc_hashmap_base *base) {
    return (void *)((unsigned char *)base + base->hm_mdoff);
}

/* scc_hashmap_emplace
 *
 * Emplace the given key-value pair in the map
 *
 * void *map
 *      Handle to the hash map
 *
 * struct scc_hashmap_base *base
 *      Base address of the map
 *
 * size_t keysize
 *      Size of the key type
 *
 * size_t valsize
 *      Size of the value type
 */
void scc_hashmap_emplace(void *map, struct scc_hashmap_base *base, size_t keysize, size_t valsize) {
    unsigned long long hash = base->hm_hash(map, keysize);
    unsigned long long index = scc_hashmap_probe_insert(base, map, keysize, hash);

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
}

/* scc_hashmap_realloc
 *
 * Allocate a new hash map, fill in the fields of
 * its header and return its base address.
 *
 * void *restrict *newmap
 *      Address of a new handle to be used for referring to the
 *      map
 *
 * void const *map
 *      Handle of the original map
 *
 * struct scc_hashmap_base const *base
 *      Base of the map referred to by handle
 *
 * size_t keysize
 *      Size of the key type
 *
 * size_t valsize
 *      Size of the value type
 *
 * size_t cap
 *      Capacity of the new map
 */
static struct scc_hashmap_base *scc_hashmap_realloc(
    void *restrict *newmap,
    void const *map,
    struct scc_hashmap_base const *base,
    size_t keysize,
    size_t valsize,
    size_t cap
) {
    assert(scc_hashmap_is_power_of_2(cap));

    /* Size of map up to and including hm_curr */
    size_t const hdrsize =
        (unsigned char const *)map - (unsigned char const *)base + base->hm_pairsize;

    /* Offset of key array, no padding possible between hm_curr
     * and key array (although struct may contain trailing bytes) */
    size_t keyoff = scc_align(hdrsize, base->hm_keyalign);
    assert((keyoff & ~(base->hm_keyalign - 1u)) == keyoff);

    /* Offset of value array */
    size_t valoff = keyoff + cap * keysize;
    valoff = scc_align(valoff, base->hm_valalign);
    assert((valoff & ~(base->hm_valalign - 1u)) == valoff);

    /* Offset of metadata array */
    size_t mdoff = valoff + cap * valsize;
    mdoff = scc_align(mdoff, scc_alignof(scc_hashmap_metatype));
    assert((mdoff & ~(scc_alignof(scc_hashmap_metatype) - 1u)) == mdoff);

    size_t const size = mdoff + (cap + SCC_HASHMAP_GUARDSZ) * sizeof(scc_hashmap_metatype);

    /* Allocate new map, ignore clang tidy being scared by scc_hashmap_base being
     * larger than unsigned char */
    struct scc_hashmap_base *newbase = calloc(size, sizeof(unsigned char )); /* NOLINT */
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

/* scc_hashmap_rehash
 *
 * Reallocate and rehash the entire hash map. On success,
 * *map is updated to refer to the new map and true is
 * returned. On failure, *map remains unchanged.
 *
 * void **map
 *      Address of the map handle
 *
 * struct scc_hashmap_base *base
 *      Base address of the hash map referred to by *map
 *
 * size_t keysize
 *      Size of the key type
 *
 * size_t valsize
 *      Size of the value type
 *
 * size_t cap
 *      Capacity of the new map
 */
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

    for(size_t i = 0u; base->hm_size && i < base->hm_capacity; ++i) {
        if(md[i] & SCC_HASHMAP_OCCUPIED) {
            /* Copy key */
            memcpy(newmap, keybase + i * keysize, keysize);
            /* Copy value */
            memcpy((unsigned char *)newmap + keysize + base->hm_valpad, valbase + i * valsize, valsize);
            scc_hashmap_emplace(newmap, newbase, keysize, valsize);
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
    scc_hashmap_emplace(*(void **)mapaddr, base, keysize, valsize);

    ++base->hm_size;
    return true;
}

void *scc_hashmap_impl_find(void *map, size_t keysize, size_t valsize) {
    struct scc_hashmap_base *base = scc_hashmap_impl_base(map);
    unsigned long long hash = base->hm_hash(map, keysize);
    long long const index = scc_hashmap_probe_find(base, map, keysize, hash);
    if(index == -1ll) {
        return 0;
    }
    assert(base->hm_size);

    assert(index >= 0ll);
    assert((size_t)index < base->hm_capacity);

    unsigned char *valbase = scc_hashmap_vals(base);
    return (void *)(valbase + index * valsize);
}
