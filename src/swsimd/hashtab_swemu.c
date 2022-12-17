#include "hashtab_swemu.h"

#include <scc/bug.h>
#include <scc/hashtab.h>

#include <limits.h>
#include <stdint.h>
#include <string.h>

#ifdef SCC_HAS_UINT64
//? .. c:type:: uint64_t scc_vectype
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     Vector type used in simd emulation
typedef uint64_t scc_vectype;
#else
//? .. c:type:: unsigned long long scc_vectype
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     Vector type used in simd emulation
typedef unsigned long long scc_vectype;
#endif

scc_static_assert(sizeof(scc_vectype) == SCC_VECSIZE, "Vector size is not 64 bits");
/* Standard mandates that UCHAR_MAX >= 255 meaning a byte must be
 * at least 8 bits */
scc_static_assert(CHAR_BIT >= 8, "Non-conformant implementation");

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
#ifdef SCC_HAS_UINT64
    scc_vectype = (uint64_t)0x0101010101010101ull
#else
    /* Cannot assume sizeof(scc_vectype) */
    scc_vectype metamask = 0u;
    for(unsigned i = 0u; i < sizeof(metamask); ++i) {
        metamask = (metamask << 8u) | 0x01u;
    }
#endif
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
//?     :returns: :code:`ldaddr` rounded down to the nearest multiple of :c:texpr:`sizeof(scc_vectype)`
static scc_vectype const *scc_hashtab_align_vecld(unsigned char const *ldaddr) {
#ifdef SCC_HAS_UINTPTR
    return (void const *)((uintptr_t)ldaddr & ~(sizeof(scc_vectype) - 1u));
#else
    unsigned char byte;
    memcpy(&byte, &ldaddr, sizeof(byte));
    unsigned char aligned = byte & ~(sizeof(scc_vectype) - 1u);
    return (void const *)(ldaddr + aligned - byte);
#endif
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

    /* Start slot */
    size_t sslot = hash & (base->ht_capacity - 1u);
    scc_vectype const *ldaddr = scc_hashtab_align_vecld(meta + sslot);

    /* Aligned offset */
    size_t slot = (unsigned char const *)ldaddr - meta;
    /* Slot adjustment for aligning */
    size_t const slot_adj = (sslot - slot);

    scc_vectype curr = *ldaddr;
    /* Rounding may have introduced matches that were to
     * be matched last, mask them out */
}
