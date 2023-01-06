#include "hashmap_swemu.h"
#include "swvec.h"

#include <scc/bug.h>
#include <scc/hashmap.h>

#include <assert.h>
#include <limits.h>

scc_static_assert(sizeof(scc_vectype) < SCC_HASHMAP_STACKCAP);

//? .. c:function:: scc_vectype scc_hashmap_gen_metamask(unsigned long long hash)
//?
//?     Broadcast high 7 bits of the hash to each byte in a vector. Set
//?     the high bit to ease comparison with ``hashmap`` meta entries
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param hash: The hash of the element
//?     :returns: A vector where each byte contains 0x80 | <7 MSB of hash>
static inline scc_vectype scc_hashmap_gen_metamask(unsigned long long hash) {
    return scc_swvec_bcast(0x80u | (hash >> (sizeof(scc_vectype) * CHAR_BIT - (CHAR_BIT - 1u))));
}

long long scc_hashmap_probe_find(
    struct scc_hashmap_base const *base,
    void const *handle,
    size_t keysize,
    unsigned long long hash
) {
    /* 7 high bits of hash, packed */
    scc_vectype metamask = scc_hashmap_gen_metamask(hash);
    /* Metadata array */
    unsigned char const *meta = (unsigned char const *)base + base->hm_mdoff;
    /* Value array */
    unsigned char const *keys = (unsigned char const *)handle + base->hm_pairsize;

    /* Start slot */
    size_t sslot = hash & (base->hm_capacity - 1u);
    scc_vectype const *ldaddr = scc_swvec_align_load(meta + sslot);

    /* Aligned offset */
    size_t start = (unsigned char const *)ldaddr - meta;
    /* Slot adjustment for aligning */
    size_t const slot_adj = (sslot - start);
    assert(slot_adj < CHAR_BIT);

    scc_vectype curr = *ldaddr;

    /* All zeroes for non-vacant with matching hash */
    scc_vectype occ_match = curr ^ metamask;
    /* All zeroes for probe end */
    scc_vectype probe_end = curr ^ 0u;

    for(unsigned i = slot_adj; i < sizeof(curr); ++i) {
        if(!scc_swvec_read_byte(probe_end, i)) {
            return -1ll;
        }
        if(!scc_swvec_read_byte(occ_match, i) && base->hm_eq(keys + (start + i) * keysize, handle)) {
            return (long long)(start + i);
        }
    }

    /* Advance and wrap */
    size_t slot = (start + sizeof(scc_vectype)) & (base->hm_capacity - 1u);

    /* Look through the bulk of the map */
    while(slot != start) {
        curr = *(scc_vectype const *)(meta + slot);
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;

        /* Check elements */
        for(unsigned i = 0u; i < sizeof(curr); ++i) {
            if(!scc_swvec_read_byte(probe_end, i)) {
                return -1ll;
            }
            if(!scc_swvec_read_byte(occ_match, i) && base->hm_eq(keys + (slot + i) * keysize, handle)) {
                return (long long)(slot + i);
            }
        }
        /* Advance */
        slot = (slot + sizeof(scc_vectype)) & (base->hm_capacity - 1u);
    }

    /* Residual */
    if(slot_adj) {
        curr = *(scc_vectype const *)(meta + slot);
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;
        for(unsigned i = 0u; i < slot_adj; ++i) {
            if(!scc_swvec_read_byte(probe_end, i)) {
                return -1ll;
            }
            if(!scc_swvec_read_byte(occ_match, i) && base->hm_eq(keys + (slot + i) * keysize, handle)) {
                return (long long)(slot + i);
            }
        }
    }

    return -1ll;
}

unsigned long long scc_hashmap_probe_insert(
    struct scc_hashmap_base const *base,
    void const *handle,
    size_t keysize,
    unsigned long long hash
) {
    /* 7 high bits of hash, packed */
    scc_vectype metamask = scc_hashmap_gen_metamask(hash);
    scc_vectype ones = ~(scc_vectype)0u;
    /* Metadata array */
    unsigned char const *meta = (unsigned char const *)base + base->hm_mdoff;
    /* Value array */
    unsigned char const *keys = (unsigned char const *)handle + base->hm_pairsize;

    /* Start slot */
    size_t sslot = hash & (base->hm_capacity - 1u);
    scc_vectype const *ldaddr = scc_swvec_align_load(meta + sslot);

    /* Aligned offset */
    size_t start = (unsigned char const *)ldaddr - meta;
    /* Slot adjustment for aligning */
    size_t const slot_adj = (sslot - start);
    assert(slot_adj < CHAR_BIT);

    scc_vectype curr = *ldaddr;

    /* MSB 1 if vacant */
    scc_vectype vacant = curr ^ ones;
    /* All zeroes for non-vacant with matching hash */
    scc_vectype occ_match = curr ^ metamask;
    /* All zeroes for probe end */
    scc_vectype probe_end = curr ^ 0u;

    unsigned long long empty_slot = ~0ull;
    for(unsigned i = slot_adj; i < sizeof(curr); ++i) {
        if(empty_slot == ~0ull) {
            if(!scc_swvec_read_byte(occ_match, i) && base->hm_eq(keys + (i + start) * keysize, handle)) {
                empty_slot = (i + start) | SCC_HASHMAP_DUPLICATE;
            }
            else if(scc_swvec_read_byte(vacant, i) & 0x80u) {
                empty_slot = i + start;
            }
        }
        if(!scc_swvec_read_byte(probe_end, i)) {
            /* Found end, done probing */
            return empty_slot;
        }
    }

    /* Advance and wrap */
    size_t slot = (start + sizeof(scc_vectype)) & (base->hm_capacity - 1u);

    /* Look through the bulk of the table */
    while(slot != start) {
        curr = *(scc_vectype const *)(meta + slot);
        vacant = curr ^ ones;
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;

        for(unsigned i = 0u; i < sizeof(curr); ++i) {
            if(empty_slot == ~0ull) {
                if(!scc_swvec_read_byte(occ_match, i) && base->hm_eq(keys + (slot + i) * keysize, handle)) {
                    return (slot + i) | SCC_HASHMAP_DUPLICATE;
                }
                else if(scc_swvec_read_byte(vacant, i) & 0x80u) {
                    empty_slot = slot + i;
                }
            }
            if(!scc_swvec_read_byte(probe_end, i)) {
                /* Found end, done probing */
                return empty_slot;
            }
        }

        /* Advance */
        slot = (slot + sizeof(scc_vectype)) & (base->hm_capacity - 1u);
    }

    /* Residual */
    if(slot_adj) {
        curr = *(scc_vectype const *)(meta + slot);
        vacant = curr ^ ones;
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;
        for(unsigned i = 0u; i < slot_adj; ++i) {
            if(empty_slot == ~0ull) {
                if(!scc_swvec_read_byte(occ_match, i) && base->hm_eq(keys + (slot + i) * keysize, handle)) {
                    return (slot + i) | SCC_HASHMAP_DUPLICATE;
                }
                else if(scc_swvec_read_byte(vacant, i) & 0x80u) {
                    empty_slot = (long long)(slot + i);
                }
            }
            if(!scc_swvec_read_byte(probe_end, i)) {
                /* Found end, done probing */
                return empty_slot;
            }
        }
    }

    assert(empty_slot != ~0ull);
    return empty_slot;
}
