#include <scc/arch.h>
#include <scc/bug.h>
#include <scc/hashtab.h>
#include <scc/swar.h>

#include <assert.h>
#include <limits.h>

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
    return scc_swar_bcast(0x80u | (hash >> (sizeof(scc_vectype) * CHAR_BIT - (CHAR_BIT - 1u))));
}

long long scc_hashtab_impl_probe_find_swar(
    struct scc_hashtab_base const *base,
    void const *handle,
    size_t elemsize,
    unsigned long long hash
) {
    scc_static_assert(sizeof(scc_vectype) < SCC_HASHTAB_STACKCAP);

    /* 7 high bits of hash, packed */
    scc_vectype metamask = scc_hashtab_gen_metamask(hash);
    /* Metadata array */
    unsigned char const *meta = (unsigned char const *)base + base->ht_mdoff;
    /* Value array */
    unsigned char const *vals = (unsigned char const *)handle + elemsize;

    /* Start slot */
    size_t sslot = hash & (base->ht_capacity - 1u);
    scc_vectype const *ldaddr = scc_swar_align_load(meta + sslot);

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

    unsigned i;
    for(i = slot_adj; i < sizeof(curr) && start + i < base->ht_capacity; ++i) {
        if(!scc_swar_read_byte(probe_end, i)) {
            return -1ll;
        }
        if(!scc_swar_read_byte(occ_match, i) && base->ht_eq(vals + (start + i) * elemsize, handle)) {
            return (long long)(start + i);
        }
    }

    /* Advance and wrap */
    size_t slot = (start + i) & (base->ht_capacity - 1u);

    /* Look through the bulk of the table */
    while(slot != start) {
        curr = *(scc_vectype const *)(meta + slot);
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;

        /* Check elements */
        for(i = 0u; i < sizeof(curr); ++i) {
            if(!scc_swar_read_byte(probe_end, i)) {
                return -1ll;
            }
            if(!scc_swar_read_byte(occ_match, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return (long long)(slot + i);
            }
        }
        /* Advance */
        slot = (slot + i) & (base->ht_capacity - 1u);
    }

    /* Residual */
    if(slot_adj) {
        curr = *(scc_vectype const *)(meta + slot);
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;
        for(i = 0u; i < slot_adj; ++i) {
            scc_when_mutating(assert(i < slot_adj));
            if(!scc_swar_read_byte(probe_end, i)) {
                return -1ll;
            }
            if(!scc_swar_read_byte(occ_match, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return (long long)(slot + i);
            }
        }
    }

    return -1ll;
}

long long scc_hashtab_impl_probe_insert_swar(
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
    scc_vectype const *ldaddr = scc_swar_align_load(meta + sslot);

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

    long long empty_slot = -1ll;
    unsigned i;
    for(i = slot_adj; i < sizeof(curr) && start + i < base->ht_capacity; ++i) {
        if(!scc_swar_read_byte(occ_match, i) && base->ht_eq(vals + (i + start) * elemsize, handle)) {
            /* Already in table */
            return -1ll;
        }
        if(empty_slot == -1ll && scc_swar_read_byte(vacant, i) & 0x80u) {
            empty_slot = (long long)(i + start);
        }
        if(!scc_swar_read_byte(probe_end, i)) {
            /* Found end, done probing */
            return empty_slot;
        }
    }

    /* Advance and wrap */
    size_t slot = (start + i) & (base->ht_capacity - 1u);

    /* Look through the bulk of the table */
    while(slot != start) {
        curr = *(scc_vectype const *)(meta + slot);
        vacant = curr ^ ones;
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;

        for(i = 0u; i < sizeof(curr); ++i) {
            if(!scc_swar_read_byte(occ_match, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return -1ll;
            }
            if(empty_slot == -1ll && scc_swar_read_byte(vacant, i) & 0x80u) {
                empty_slot = (long long)(slot + i);
            }
            if(!scc_swar_read_byte(probe_end, i)) {
                /* Found end, done probing */
                return empty_slot;
            }
        }

        /* Advance */
        slot = (slot + i) & (base->ht_capacity - 1u);
    }

    /* Residual */
    if(slot_adj) {
        curr = *(scc_vectype const *)(meta + slot);
        vacant = curr ^ ones;
        occ_match = curr ^ metamask;
        probe_end = curr ^ 0u;
        for(i = 0u; i < slot_adj; ++i) {
            scc_when_mutating(assert(i < slot_adj));
            if(!scc_swar_read_byte(occ_match, i) && base->ht_eq(vals + (slot + i) * elemsize, handle)) {
                return -1ll;
            }
            if(empty_slot == -1ll && scc_swar_read_byte(vacant, i) & 0x80u) {
                empty_slot = (long long)(slot + i);
            }
            if(!scc_swar_read_byte(probe_end, i)) {
                /* Found end, done probing */
                return empty_slot;
            }
        }
    }

    assert(empty_slot != -1ll);
    return empty_slot;
}
