#include <scc/bloom.h>
#include <scc/bug.h>
#include <scc/mem.h>

#ifdef SCC_HAVE_LIBM
#include <math.h>
#endif
#include <stdbool.h>
#include <stdlib.h>

#if defined SCC_HAVE_UINT32_T || defined SCC_HAVE_UINT64_T

size_t scc_bloom_impl_npad(void const *flt);
size_t scc_bloom_capacity(void const *flt);
size_t scc_bloom_nhashes(void const *flt);

static inline bool scc_bloom_is_allocd(void const *flt) {
    return ((unsigned char const *)flt)[-1];
}

void *scc_bloom_impl_with_hash(struct scc_bloom_base *base, size_t offset,
        unsigned m, unsigned k, scc_bloom_hash hash) {
    base->bm_hash = hash;
    base->bm_nbits = m ? ((m + 7u) & ~7u) : 8u;
    base->bm_nhashes = k ? k : 4u;

    unsigned char *tmp = (unsigned char *)base + offset;
    tmp[-2] = offset - sizeof(*base) - (sizeof(*tmp) << 1u);

    return tmp;
}

void *scc_bloom_impl_new(struct scc_bloom_base *base, size_t offset, unsigned m, unsigned k) {
    return scc_bloom_impl_with_hash(base, offset, m, k, scc_hash_murmur128);
}

void *scc_bloom_impl_with_hash_dyn(size_t size, size_t offset, unsigned m,
        unsigned k, scc_bloom_hash hash) {
    struct scc_bloom_base *base = calloc(1u, size);
    if (!base)
        return 0;

    unsigned char *tmp = scc_bloom_impl_with_hash(base, offset, m, k, hash);
    tmp[-1] = 1;
    return tmp;
}

void *scc_bloom_impl_new_dyn(size_t size, size_t offset, unsigned m, unsigned k) {
    return scc_bloom_impl_with_hash_dyn(size, offset, m, k, scc_hash_murmur128);
}

void scc_bloom_free(void *flt) {
    struct scc_bloom_base *base = scc_bloom_impl_base(flt);
    if (scc_bloom_is_allocd(flt))
        free(base);
}

static inline unsigned char *scc_bloom_bitset(void *flt, size_t elemsize) {
    return (unsigned char *)flt + elemsize;
}

static inline void scc_bloom_set_bit(unsigned char *bitset, unsigned bitidx) {
    bitset[bitidx >> 3u] |= (1u << (bitidx & 7u));
}

static inline _Bool scc_bloom_bit_is_set(unsigned char const *bitset, unsigned bitidx) {
    return bitset[bitidx >> 3u] & (1u << (bitidx & 7u));
}

void scc_bloom_impl_insert(void *flt, size_t elemsize) {
    struct scc_bloom_base *base = scc_bloom_impl_base(flt);

    unsigned char *bitset = scc_bloom_bitset(flt, elemsize);

    struct {
        /* Use 4-ish bytes of stack to allow accessing digest
         * in chunks of 4 bytes */
        uint_fast32_t force_align;
        struct scc_digest128 d;
    } m;

    unsigned i;
    for (i = 0u; i < base->bm_nhashes >> 2u; ++i) {
        scc_static_assert(scc_alignof(struct scc_digest128) == 1u);
        base->bm_hash(&m.d, flt, elemsize, i);

        scc_bloom_set_bit(bitset, *(uint_fast32_t *)m.d.digest % base->bm_nbits);
        scc_bloom_set_bit(bitset, *(uint_fast32_t *)(m.d.digest + 4u) % base->bm_nbits);
        scc_bloom_set_bit(bitset, *(uint_fast32_t *)(m.d.digest + 8u) % base->bm_nbits);
        scc_bloom_set_bit(bitset, *(uint_fast32_t *)(m.d.digest + 12u) % base->bm_nbits);
    }

    if (base->bm_nhashes & 3u) {
        base->bm_hash(&m.d, flt, elemsize, i);
        for (unsigned j = 0u; j < (base->bm_nhashes & 3u); ++j)
            scc_bloom_set_bit(bitset, *(uint_fast32_t *)(m.d.digest + (j << 2u)) % base->bm_nbits);
    }
}

_Bool scc_bloom_impl_test(void *flt, size_t elemsize) {
    struct scc_bloom_base *base = scc_bloom_impl_base(flt);
    bool present = true;

    unsigned char *bitset = scc_bloom_bitset(flt, elemsize);

    struct {
        uint_fast32_t force_align;
        struct scc_digest128 d;
    } m;

    unsigned i;
    for (i = 0u; i < base->bm_nhashes >> 2u && present; ++i) {
        scc_static_assert(scc_alignof(struct scc_digest128) == 1u);
        base->bm_hash(&m.d, flt, elemsize, i);

        present &= scc_bloom_bit_is_set(bitset, *(uint_fast32_t *)m.d.digest % base->bm_nbits);
        present &= scc_bloom_bit_is_set(bitset, *(uint_fast32_t *)(m.d.digest + 4u) % base->bm_nbits);
        present &= scc_bloom_bit_is_set(bitset, *(uint_fast32_t *)(m.d.digest + 8u) % base->bm_nbits);
        present &= scc_bloom_bit_is_set(bitset, *(uint_fast32_t *)(m.d.digest + 12u) % base->bm_nbits);
    }

    if (present && base->bm_nhashes & 3u) {
        base->bm_hash(&m.d, flt, elemsize, i);
        for (unsigned j = 0u; j < (base->bm_nhashes & 3u) && present; ++j)
            present &= scc_bloom_bit_is_set(bitset, *(uint_fast32_t *)(m.d.digest + (j << 2u)) % base->bm_nbits);
    }

    return present;
}

#ifdef SCC_HAVE_LIBM
size_t scc_bloom_impl_size(void const *flt, size_t elemsize) {
    struct scc_bloom_base const *base = scc_bloom_impl_base_qual(flt, const);
    double m = base->bm_nbits;
    double k = base->bm_nhashes;

    unsigned char const *bitset = (unsigned char const *)flt + elemsize;
    unsigned x = 0u;
    for (unsigned i = 0u; i < base->bm_nbits >> 3u; ++i) {
        x += !!(bitset[i] & 0x80);
        x += !!(bitset[i] & 0x40);
        x += !!(bitset[i] & 0x20);
        x += !!(bitset[i] & 0x10);
        x += !!(bitset[i] & 0x08);
        x += !!(bitset[i] & 0x04);
        x += !!(bitset[i] & 0x02);
        x += !!(bitset[i] & 0x01);
    }

    double sz = round(-1.0 * m / k * log(1.0 - x / m));
    return sz < 0.0 ? 0u : (size_t)sz;
}
#endif /* SCC_HAVE_LIBM */

#endif /* SCC_HAVE_UINT32_T || SCC_HAVE_UINT64_T */
