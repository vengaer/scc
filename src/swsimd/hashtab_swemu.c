#include "hashtab_swemu.h"

#include <scc/bug.h>
#include <scc/hashtab.h>

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

//? .. c:type:: unsigned long long scc_vectype
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     Vector type used in simd emulation
typedef unsigned long long scc_vectype;

scc_static_assert(sizeof(scc_vectype) == SCC_VECSIZE, "Unexpected vector size");
/* Standard mandates that UCHAR_MAX >= 255 meaning a byte must be
 * at least 8 bits */
scc_static_assert(CHAR_BIT >= 8, "Non-conformant implementation");

scc_static_assert(sizeof(scc_vectype) < SCC_HASHTAB_STATIC_CAPACITY);

//? .. c:function:: unsigned char read_byte(scc_vectype vec, unsigned i)
//?
//?     Read the ith byte in the given vector
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: The vector to read from
//?     :param i: The index of the byte to read
//?     :returns: The ith byte of the given vector
static inline unsigned char read_byte(scc_vectype vec, unsigned i) {
    return (vec >> i * CHAR_BIT) & UCHAR_MAX;
}

//? .. c:function:: scc_vectype scc_hashtab_gen_metamask(unsigned long long hash)
//?
//?     Broadcast high 7 bits of the hash to each byte in a vector. Set
//?     the high bit to ease comparison with ``hashtab`` meta entries
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param hash: The hash of the element
//?     :returns: A vector where each byte contains 0x80 | <7 MSB of hash>
static inline scc_vectype scc_hashtab_gen_metamask(unsigned long long hash) {
    /* Cannot assume sizeof(scc_vectype) */
    scc_vectype metamask = 0u;
    for(unsigned i = 0u; i < sizeof(metamask); ++i) {
        metamask = (metamask << 8u) | 0x01u;
    }
    unsigned char meta = 0x80 | (hash >> (sizeof(metamask) * CHAR_BIT - (CHAR_BIT - 1u)));

    /* Broadcast metabyte to all bytes in the vector */
    return metamask * meta;
}

//? .. c:function:: scc_vectype const *scc_hashtab_align_vecld(unsigned char const *ldaddr)
//?
//?     Align :code:`ldaddr` down to the nearest multiple of :c:texpr:`sizeof(scc_vectype)`
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param ldaddr: The address to be aligned
//?     :returns: :code:`ldaddr` rounded down to the nearest multiple
//?               of :c:texpr:`sizeof(scc_vectype)`
static scc_vectype const *scc_hashtab_align_vecld(unsigned char const *ldaddr) {
    unsigned char byte;
    memcpy(&byte, &ldaddr, sizeof(byte));
    unsigned char aligned = byte & ~(sizeof(scc_vectype) - 1u);
    return (void const *)(ldaddr + aligned - byte);
}

long long scc_hashtab_probe_find(
    struct scc_hashtab_base const *base,
    void const *handle,
    size_t elemsize,
    unsigned long long hash
) {
    /* 7 high bits of hash, packed */
    scc_vectype metamask = scc_hashtab_gen_metamask(hash);
    /* Metadata array */
    unsigned char const *meta = (unsigned char const *)base + base->ht_mdoff;
    /* Value array */
    unsigned char const *vals = (unsigned char const *)handle + elemsize;

    /* Start slot */
    size_t sslot = hash & (base->ht_capacity - 1u);
    scc_vectype const *ldaddr = scc_hashtab_align_vecld(meta + sslot);

    /* Aligned offset */
    size_t start = (unsigned char const *)ldaddr - meta;
    /* Slot adjustment for aligning */
    size_t const slot_adj = (sslot - start);
    assert(slot_adj < CHAR_BIT);

    scc_vectype curr = *ldaddr;

    /* Compare, matching bytes are all zeroes */
    curr ^= metamask;
    for(unsigned i = slot_adj; i < sizeof(curr); ++i) {
        if(!read_byte(curr, i) && base->ht_eq(vals + (start + i) * elemsize, handle)) {
            return (long long)(start + i);
        }
    }

    /* Advance and wrap */
    size_t slot = (start + sizeof(scc_vectype)) & (base->ht_capacity - 1u);

    /* Look through the bulk of the table */
    while(slot != start) {
        curr = *(scc_vectype const *)(meta + slot) ^ metamask;

        /* Check elements */
        for(unsigned i = 0u; i < sizeof(curr); ++i) {
            if(!read_byte(curr, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return (long long)(slot + i);
            }
        }
        /* Advance */
        slot = (slot + sizeof(scc_vectype)) & (base->ht_capacity - 1u);
    }

    /* Residual */
    if(slot_adj) {
        curr = *(scc_vectype const *)(meta + slot) ^ metamask;
        for(unsigned i = 0u; i < slot_adj; ++i) {
            if(!read_byte(curr, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return (long long)(slot + i);
            }
        }
    }

    return -1ll;
}

long long scc_hashtab_probe_insert(
    struct scc_hashtab_base const *base,
    void const *handle,
    size_t elemsize,
    unsigned long long hash
) {
    /* 7 high bits of hash, packed */
    scc_vectype metamask = scc_hashtab_gen_metamask(hash);
    scc_vectype ones = ~(scc_vectype)0u;
    /* Metadata array */
    unsigned char const *meta = (unsigned char const *)base + base->ht_mdoff;
    /* Value array */
    unsigned char const *vals = (unsigned char const *)handle + elemsize;

    /* Start slot */
    size_t sslot = hash & (base->ht_capacity - 1u);
    scc_vectype const *ldaddr = scc_hashtab_align_vecld(meta + sslot);

    /* Aligned offset */
    size_t start = (unsigned char const *)ldaddr - meta;
    /* Slot adjustment for aligning */
    size_t const slot_adj = (sslot - start);
    assert(slot_adj < CHAR_BIT);

    scc_vectype curr = *ldaddr;
    /* Rounding may have caused matches to wrap, treat them as occupied */
    curr |= ~((~(scc_vectype)0u) << slot_adj);

    /* MSB 1 if vacant */
    scc_vectype vacant = curr ^ ones;
    /* All zeroes for non-vacant with matching hash */
    scc_vectype occ_match = curr ^ metamask;
    /* All zeroes for probe end */
    scc_vectype probe_end = curr ^ 0u;

    long long empty_slot = -1ll;
    for(unsigned i = slot_adj; i < sizeof(curr); ++i) {
        if(!read_byte(occ_match, i) && base->ht_eq(vals + (i + start) * elemsize, handle)) {
            /* Already in table */
            return -1ll;
        }
        if(empty_slot == -1ll && read_byte(vacant, i) & 0x80u) {
            empty_slot = (long long)(i + start);
        }
        if(!read_byte(probe_end, i)) {
            /* Found end, done probing */
            return empty_slot;
        }
    }

    /* Advance and wrap */
    size_t slot = (start + sizeof(scc_vectype)) & (base->ht_capacity - 1u);

    /* Look through the bulk of the table */
    while(slot != start) {
        curr = *(scc_vectype const *)(meta + slot);
        vacant = curr ^ ones;
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;

        for(unsigned i = 0u; i < sizeof(curr); ++i) {
            if(!read_byte(occ_match, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return -1ll;
            }
            if(empty_slot == -1ll && read_byte(vacant, i) & 0x80u) {
                empty_slot = (long long)(slot + i);
            }
            if(!read_byte(probe_end, i)) {
                /* Found end, done probing */
                return empty_slot;
            }
        }

        /* Advance */
        slot = (slot + sizeof(scc_vectype)) & (base->ht_capacity - 1u);
    }

    /* Residual */
    if(slot_adj) {
        curr = *(scc_vectype const *)(meta + slot);
        vacant = curr ^ ones;
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;
        for(unsigned i = 0u; i < slot_adj; ++i) {
            if(!read_byte(occ_match, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return -1ll;
            }
            if(empty_slot == -1ll && read_byte(vacant, i) & 0x80u) {
                empty_slot = (long long)(slot + i);
            }
            if(!read_byte(probe_end, i)) {
                /* Found end, done probing */
                return empty_slot;
            }
        }
    }

    return -1ll;
}
