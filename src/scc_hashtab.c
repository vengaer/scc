#include <scc/scc_dbg.h>
#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>
#include <scc/scc_perf.h>
#include "asm/asm_generic.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_HASHTAB_OCCUPIED = 0x80 };
enum { SCC_HASHTAB_VACATED = 0x7f };
enum { SCC_HASHTAB_HASHSHIFT = 57 };

#define scc_hashtab_is_power_of_2(val) \
    (((val) & ~((val) - 1)) == (val))

size_t scc_hashtab_capacity(void const *handle);
size_t scc_hashtab_size(void const *handle);
size_t scc_hashtab_impl_bkpad(void const *handle);

/* scc_hashtab_next_power_of_2
 *
 * Round given value up to the next power of 2
 *
 * unsigned long long val
 *      Value to round up
 */
static inline unsigned long long scc_hashtab_next_power_of_2(unsigned long long val) {
    --val;
    val |= val >> 1ull;
    val |= val >> 2ull;
    val |= val >> 4ull;
    val |= val >> 8ull;
    val |= val >> 16ull;
    val |= val >> 32ull;
    return val + 1ull;
}

/* scc_hashtab_set_mdent
 *
 * Set metadata entry at given index, duplicating
 * low entries in the guard
 *
 * scc_hashtab_metatype *md
 *      Address of the first element in the metadata
 *      array
 *
 * size_t index
 *      The index in which the value is to be set
 *
 * scc_hashtab_metatype val
 *      The value to write to the given index
 *
 * size_t capacity
 *      Table capacity for calculating guard offset
 */
static inline void scc_hashtab_set_mdent(
    scc_hashtab_metatype *md,
    size_t index,
    scc_hashtab_metatype val,
    size_t capacity
) {
    md[index] = val;
    if(index < SCC_HASHTAB_GUARDSZ) {
        md[index + capacity] = val;
    }
}

/* scc_hashtab_calcpad
 *
 * Calculate the number of padding bytes between ht_fwoff and ht_curr
 *
 * size_t coff
 *      Offset of ht_curr relative the base of the struct
 */
static inline unsigned char scc_hashtab_calcpad(size_t coff) {
    size_t const fwoff = coff -
        offsetof(struct scc_hashtab_base, ht_fwoff) -
        sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
    assert(fwoff <= UCHAR_MAX);
    return fwoff;
}

/* scc_hashtab_set_bkoff
 *
 * Set the ht_bkoff field
 *
 * void *handle
 *      Handle referring to the hash table
 *
 * unsigned char bkoff
 *      The value to set
 */
static inline void scc_hashtab_set_bkoff(void *handle, unsigned char bkoff) {
    ((unsigned char *)handle)[-1] = bkoff;
}

/* scc_hashtab_should_rehash
 *
 * Return true if the load factor is large enough to merit a rehash
 *
 * struct scc_hashtab_base const *base
 *      Address of the hash table base
 */
static inline bool scc_hashtab_should_rehash(struct scc_hashtab_base const *base) {
    /* Rehash at 87.5% */
    return base->ht_size > (base->ht_capacity >> 1u) +
                           (base->ht_capacity >> 2u) +
                           (base->ht_capacity >> 3u);
}

/* scc_hashtab_sizeup
 *
 * Compute capacity after resize for given hash table
 *
 * struct scc_hashtab_base const *base
 *      Address of the hash table base
 */
static inline size_t scc_hashtab_sizeup(struct scc_hashtab_base const *base) {
    return base->ht_capacity << 1u;
}

/* scc_hashtab_metadata
 *
 * Return address of first element of the metadata array in *base
 *
 * struct scc_hashtab_base *base
 *      Address of the hash table base
 */
static inline scc_hashtab_metatype *scc_hashtab_metadata(struct scc_hashtab_base *base) {
    return (void *)((unsigned char *)base + base->ht_mdoff);
}

/* scc_hashtab_emplace
 *
 * Attempt to emplace the value referred to by the given handle
 * into the hash table. Return true if the value was emplaced
 *
 * void *handle
 *      Handle to the hash table
 *
 * struct scc_hashtab_base *base
 *      Base address of the hash table referred to by handle
 *
 * size_t elemsize
 *      The size of the elements stored in the table
 */
static bool scc_hashtab_emplace(void *handle, struct scc_hashtab_base *base, size_t elemsize) {
    unsigned long long const hash = base->ht_hash(handle, elemsize);
    long long index = scc_hashtab_probe_insert(base, handle, elemsize, hash);

    SCC_ON_PERFTRACK(++base->ht_perf.ev_n_hash);

    if(index == -1ll) {
        return false;
    }

    assert(index >= 0ll && (size_t)index < base->ht_capacity);
    /* handle holds address of base->ht_data[-1] */
    memcpy((unsigned char *)handle + (index + 1ll) * elemsize, handle, elemsize);

    scc_hashtab_metatype *md = scc_hashtab_metadata(base);
    /* Mark slot as occupied */
    scc_hashtab_metatype ent = (scc_hashtab_metatype)(SCC_HASHTAB_OCCUPIED | (hash >> SCC_HASHTAB_HASHSHIFT));
    scc_hashtab_set_mdent(md, index, ent, base->ht_capacity);
    return true;
}

/* scc_hashtab_realloc
 *
 * Allocate a new hash table, fill in the fields of
 * its header and return its base address.
 *
 * void *restrict *newtab
 *      Address of a new handle to be used for referring to the
 *      table
 *
 * void const *handle
 *      Handle of the original hashtable
 *
 * struct scc_hashtab_base const *base
 *      Base of the table referred to by handle
 *
 * size_t elemsize
 *      Size of the elements to be stored in the new table
 *
 * size_t cap
 *      Capacity of the new table
 */
static struct scc_hashtab_base *scc_hashtab_realloc(
    void *restrict *newtab,
    void const *handle,
    struct scc_hashtab_base const *base,
    size_t elemsize,
    size_t cap
) {
    assert(scc_hashtab_is_power_of_2(cap));

    /* Size of table up to and including ht_curr */
    size_t const hdrsize =
        (unsigned char const *)handle - (unsigned char const *)base + elemsize;

    /* Size of ht_data for new table */
    size_t const datasize = cap * elemsize;
    size_t const align = scc_alignof(scc_hashtab_metatype);

    /* Initial metadata offset, no padding allowed between
     * ht_curr and ht_data */
    size_t mdoff = hdrsize + datasize;

    /* Align metadata */
    mdoff = (mdoff + align - 1) & ~(align - 1);
    assert((mdoff & ~(align - 1)) == mdoff);

    size_t const size = mdoff + (cap + SCC_HASHTAB_GUARDSZ) * sizeof(scc_hashtab_metatype);

    /* Allocate new hash table
     * Ignore clang tidy complaining about struct scc_hashtab_base being
     * larger than unsigned char */
    struct scc_hashtab_base *newbase = calloc(size, sizeof(unsigned char)); /* NOLINT */
    if(!newbase) {
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

/* scc_hashtab_rehash
 *
 * Reallocate and rehash the entire hash table. On success,
 * *handle is updated to refer to the new table and true is
 * returned. On failure, *handle remains unchanged.
 *
 * void **handle
 *      Address of the handle used to refer to hash table
 *
 * struct scc_hashtab_base *base
 *      Base address of the hash table referred to by *handle
 *
 * size_t elemsize
 *      Size of each element stored in the table
 *
 * size_t cap
 *      Capacity of the new table
 */
static bool scc_hashtab_rehash(
    void **handle,
    struct scc_hashtab_base *base,
    size_t elemsize,
    size_t cap
) {
    void *newtab;
    struct scc_hashtab_base *newbase = scc_hashtab_realloc(&newtab, *handle, base, elemsize, cap);
    if(!newbase) {
        return false;
    }

    scc_hashtab_metatype *md = scc_hashtab_metadata(base);

    for(size_t i = 0u; base->ht_size && i < base->ht_capacity; ++i) {
        if(md[i] & SCC_HASHTAB_OCCUPIED) {
            memcpy(newtab, (unsigned char *)*(void **)handle + (i + 1u) * elemsize, elemsize);
            scc_bug_on(!scc_hashtab_emplace(newtab, newbase, elemsize));
            --base->ht_size;
        }
    }

    SCC_ON_PERFTRACK(++newbase->ht_perf.ev_n_rehashes);

    /* Copy ht_curr of old table */
    memcpy(newtab, *handle, elemsize);
    scc_hashtab_free(*handle);
    *handle = newtab;
    return true;
}

bool scc_hashtab_impl_insert(void *handleaddr, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(*(void **)handleaddr);
    if(scc_hashtab_should_rehash(base)) {
        size_t const newcap = scc_hashtab_sizeup(base);
        if(!scc_hashtab_rehash(handleaddr, base, elemsize, newcap)) {
            return false;
        }

        /* Table has been reallocated */
        base = scc_hashtab_impl_base(*(void **)handleaddr);
    }
    if(!scc_hashtab_emplace(*(void **)handleaddr, base, elemsize)) {
        return false;
    }

    SCC_ON_PERFTRACK(++base->ht_perf.ev_n_inserts);
    ++base->ht_size;
    return true;
}

void const *scc_hashtab_impl_find(void const *handle, size_t elemsize) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(handle, const);
    unsigned long long const hash = base->ht_hash(handle, elemsize);
    long long const index = scc_hashtab_probe_find(base, handle, elemsize, hash);
    if(index == -1ll) {
        return 0;
    }
    assert(base->ht_size);
    assert(index >= 0ll && (size_t)index < base->ht_capacity);

    /* handle holds &base->ht_data[-1] */
    return (void const *)((unsigned char const *)handle + (index + 1ull) * elemsize);
}

void *scc_hashtab_impl_init(struct scc_hashtab_base *base, scc_eq eq, scc_hash hash, size_t coff, size_t mdoff) {
    base->ht_eq = eq;
    base->ht_hash = hash;
    base->ht_mdoff = mdoff;
    base->ht_capacity = SCC_HASHTAB_STACKCAP;
    base->ht_fwoff = scc_hashtab_calcpad(coff);
    unsigned char *tab = (unsigned char *)base + coff;
    SCC_ON_PERFTRACK(
        base->ht_perf.ev_bytesz = mdoff + (SCC_HASHTAB_STACKCAP + SCC_HASHTAB_GUARDSZ) * sizeof(scc_hashtab_metatype)
    );
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

bool scc_hashtab_impl_reserve(void *handleaddr, size_t capacity, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(*(void **)handleaddr);
    if(capacity <= base->ht_capacity) {
        return true;
    }
    if(!scc_hashtab_is_power_of_2(capacity)) {
        capacity = scc_hashtab_next_power_of_2(capacity);
    }
    if(!scc_hashtab_rehash(handleaddr, base, elemsize, capacity)) {
        return false;
    }

    return true;
}

bool scc_hashtab_impl_remove(void *handle, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(handle);
    unsigned long long const hash = base->ht_hash(handle, elemsize);

    long long const index = scc_hashtab_probe_find(base, handle, elemsize, hash);
    if(index == -1ll) {
        return false;
    }

    assert(base->ht_size);
    assert(index >= 0ll && (size_t)index < base->ht_capacity);

    scc_hashtab_metatype *md = scc_hashtab_metadata(base);
    scc_hashtab_set_mdent(md, index, SCC_HASHTAB_VACATED, base->ht_capacity);
    --base->ht_size;
    return true;
}

void scc_hashtab_clear(void *handle) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(handle);
    scc_hashtab_metatype *md = scc_hashtab_metadata(base);
    memset(md, 0, (base->ht_capacity + SCC_HASHTAB_GUARDSZ) * sizeof(*md));
    base->ht_size = 0u;
}
