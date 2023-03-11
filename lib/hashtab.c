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

//? .. c:function:: void scc_hashtab_set_mdend(scc_hashtab_metatype *md, size_t index, scc_hashtab_metatype val, size_t capacity)
//?
//?     Set metadata entry at given index, duplicating low entries in the guarg
//?
//?     .. note::
//?
//?        Internal use only
//?
//?     :param md: Address of the first element in the metadata array
//?     :param index: The index in which the value is to be set
//?     :param val: The value to write at specified index
//?     :param capacity: Table capacity at the time of the call
static inline void scc_hashtab_set_mdent(scc_hashtab_metatype *md, size_t index, scc_hashtab_metatype val, size_t capacity) {
    md[index] = val;
    if(index < SCC_HASHTAB_GUARDSZ) {
        md[index + capacity] = val;
    }
}

//? .. c:function:: unsigned char scc_hashtab_calcpad(size_t coff)
//?
//?     Calculate the number of padding b ytes between :ref:`ht_fwoff <unsigned_char_ht_fwoff>`
//?     and :ref:`ht_curr <type_ht_curr>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param coff: Offset of :code:`ht_curr` relative the base of the hash table
//?     :returns: The number of padding bytes between :code:`ht_fwoff` and :code:`ht_curr`
static inline unsigned char scc_hashtab_calcpad(size_t coff) {
    size_t const fwoff = coff -
        offsetof(struct scc_hashtab_base, ht_fwoff) -
        sizeof(((struct scc_hashtab_base *)0)->ht_fwoff);
    assert(fwoff <= UCHAR_MAX);
    return fwoff;
}

//? .. c:function:: void scc_hashtab_set_bkoff(void *tab, unsigned char bkoff)
//?
//?     Set ht :ref:`ht_bkoff <unsigned_char_ht_bkoff>` field
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tab: Handle referring to the hash table
//?     :param bkoff: The value to write to the field
static inline void scc_hashtab_set_bkoff(void *tab, unsigned char bkoff) {
    ((unsigned char *)tab)[-1] = bkoff;
}

//? .. c:function:: _Bool scc_hashtab_should_rehash(struct scc_hashtab_base const *base)
//?
//?     Determine whether the hash table needs to be rehashed to accomodate for
//?     potential insertion.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of the base structure of the hash table
//?     :returns: A :code:`_Bool` indicating whether the hash table should be rehashed or not
static inline bool scc_hashtab_should_rehash(struct scc_hashtab_base const *base) {
    /* Rehash at 87.5% */
    return base->ht_size > (base->ht_capacity >> 1u) +
                           (base->ht_capacity >> 2u) +
                           (base->ht_capacity >> 3u);
}

//? .. c:function:: size_t scc_hashtab_sizeup(struct scc_hashtab_base const *base)
//?
//?     Compute the capacity to set at next rehash
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of the base structure of the hash table
//?     :returns: The capacity of the table after the next rehash
static inline size_t scc_hashtab_sizeup(struct scc_hashtab_base const *base) {
    return base->ht_capacity << 1u;
}

//? .. c:function:: scc_hashtab_metatype *scc_hashtab_metadata(struct scc_hashtab_base *base)
//?
//?     Compute and return address of first element in the metadata array
//?     of the given hash table
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Address of the base structure of the hash table
//?     :returns: Address of the first element in the metadata array of :code:`*base`
static inline scc_hashtab_metatype *scc_hashtab_metadata(struct scc_hashtab_base *base) {
    return (void *)((unsigned char *)base + base->ht_mdoff);
}

//? .. c:function:: _Bool scc_hashtab_emplace(void *tab, struct scc_hashtab_base *base, size_t elemsize)
//?
//?     Attempt to emplace the value stored in the :ref:`ht_curr <type_ht_curr>` field of the
//?     given hash table.
//?
//?     :code:`tab` and :code:`base` refer to the same hash table and pointer derived from them
//?     may therefore alias.
//?
//?     :param tab: Handle to the hash table
//?     :param base: Base address of the hash table
//?     :param elemsize: Size of the elements stored in the table
//?     :returns: A :code:`_Bool` indicating whether emplacement was successful
//?     :retval true: The element was successfully emplaced
//?     :retval false: The hash table already contains the element in question
static bool scc_hashtab_emplace(void *tab, struct scc_hashtab_base *base, size_t elemsize) {
    unsigned long long const hash = base->ht_hash(tab, elemsize);
    long long index = scc_hashtab_impl_probe_insert(base, tab, elemsize, hash);

    SCC_ON_PERFTRACK(++base->ht_perf.ev_n_hash);

    if(index == -1ll) {
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

//? .. c:function:: struct scc_hashtab_base *scc_hashtab_realloc(void *restrict *newtab, void const *tab, \
//?                                                              struct scc_hashtab_base const *base, size_t elemsize, size_t cap)
//?
//?     Allocate a new hash table, fill in the fields of its base struct and
//?     return the base address.
//?
//?     The :code:`tab` and :code:`base` addresses refer to the same table and pointers derived from them
//?     might therefore alias.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param newtab: Address of the handle to be used for referring to the newly allocated table
//?     :param tab: Handle to the original hash table
//?     :param base: Base address of the original table
//?     :param elemsize: Size of the elements stored in the tabled
//?     :param cap: Capacity of the new table
//?     :returns: Base address of the newly allocated hash table, or :code:`NULL` on failure.
//?     :retval NULL: Memory allocation failed
//?     :retval Valid address: Base address of the newly allocated hash table
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
    /* Kill mutant */
    assert(size > mdoff + cap);
#ifdef SCC_CANARY_ENABLED
    size += SCC_HASHTAB_CANARYSZ;
#endif

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

//? .. c:function:: _Bool scc_hashtab_rehash(void **tab, struct scc_hashtab_base *base, size_t elemsize, size_t cap)
//?
//?     Reallocate and rehash the given hash table, On success :c:expr:`*tab` is updated to refer to the
//?     newly allocated table. If the rehashing fails, :c:expr:`*tab` remains unchanged.
//?
//?     :c:expr:`*tab` and :c:expr:`base` refer to the same hash table and pointers derived from
//?     them may therefore alias.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param tab: Address of the handle used to refer to the hash table
//?     :param base: Base address of the hash table
//?     :param elemsize: Size of each element in the hash table
//?     :param cap: Capacity of the new table. Must be a power of 2.
static bool scc_hashtab_rehash(void **tab, struct scc_hashtab_base *base, size_t elemsize, size_t cap) {
    void *newtab;
    struct scc_hashtab_base *newbase = scc_hashtab_realloc(&newtab, *tab, base, elemsize, cap);
    if(!newbase) {
        return false;
    }

    scc_hashtab_metatype *md = scc_hashtab_metadata(base);

    for(size_t i = 0u; base->ht_size; ++i) {
        assert(i < base->ht_capacity);
        if(md[i] & SCC_HASHTAB_OCCUPIED) {
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

bool scc_hashtab_impl_insert(void *tabaddr, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(*(void **)tabaddr);
    if(scc_hashtab_should_rehash(base)) {
        size_t const newcap = scc_hashtab_sizeup(base);
        if(!scc_hashtab_rehash(tabaddr, base, elemsize, newcap)) {
            return false;
        }

        /* Table has been reallocated */
        base = scc_hashtab_impl_base(*(void **)tabaddr);
    }
    if(!scc_hashtab_emplace(*(void **)tabaddr, base, elemsize)) {
        return false;
    }

    SCC_ON_PERFTRACK(++base->ht_perf.ev_n_inserts);
    ++base->ht_size;
    return true;
}

void const *scc_hashtab_impl_find(void const *tab, size_t elemsize) {
    struct scc_hashtab_base const *base = scc_hashtab_impl_base_qual(tab, const);
    if(!base->ht_size) {
        return 0;
    }
    unsigned long long const hash = base->ht_hash(tab, elemsize);
    long long const index = scc_hashtab_impl_probe_find(base, tab, elemsize, hash);
    if(index == -1ll) {
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

void scc_hashtab_free(void *tab) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);
    if(base->ht_dynalloc) {
        free(base);
    }
}

bool scc_hashtab_impl_reserve(void *tabaddr, size_t capacity, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(*(void **)tabaddr);
    if(capacity <= base->ht_capacity) {
        return true;
    }
    if(!scc_bits_is_power_of_2(capacity)) {
        unsigned n;
        for(n = 0u; capacity; capacity >>= 1u, ++n);
        capacity = 1u << n;
    }
    assert(scc_bits_is_power_of_2(capacity));
    if(!scc_hashtab_rehash(tabaddr, base, elemsize, capacity)) {
        return false;
    }

    return true;
}

bool scc_hashtab_impl_remove(void *tab, size_t elemsize) {
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);
    if(!base->ht_size) {
        return false;
    }

    unsigned long long const hash = base->ht_hash(tab, elemsize);

    long long const index = scc_hashtab_impl_probe_find(base, tab, elemsize, hash);
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
    /* Kill mutant */
    assert(sz > obase->ht_mdoff + obase->ht_capacity);
#ifdef SCC_CANARY_ENABLED
    sz += SCC_HASHTAB_CANARYSZ;
#endif
    struct scc_hashtab_base *nbase = malloc(sz);
    if(!nbase) {
        return 0;
    }
    scc_memcpy(nbase, obase, sz);
    nbase->ht_dynalloc = 1;
    return (unsigned char *)nbase + offsetof(struct scc_hashtab_base, ht_fwoff) + nbase->ht_fwoff + sizeof(nbase->ht_fwoff);
}
