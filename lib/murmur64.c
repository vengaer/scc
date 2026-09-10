/* 64 bit murmur3 hash. Original available at
 * https://github.com/aappleby/smhasher */

#include <scc/bug.h>
#include <scc/hash.h>
#include <scc/mem.h>

#include <assert.h>
#include <string.h>

#define SCC_MURMUR64_C1 UINT64_C(0x87c37b91114253d5)
#define SCC_MURMUR64_C2 UINT64_C(0x4cf5ad432745937f)

#ifdef SCC_HAVE_UINT64_T

static inline uint_fast64_t scc_rol64(uint64_t value, uint_fast8_t by) {
    return (value << by) | (value >> (64u - by));
}

static inline uint_fast64_t scc_fmix64(uint64_t h) {
    h ^= h >> 33u;
    h *= UINT64_C(0xff51afd7ed558ccd);
    h ^= h >> 33u;
    h *= UINT64_C(0xc4ceb9fe1a85ec53);
    h ^= h >> 33u;
    return h;
}

static inline void scc_murmur64_128_main_calc(uint64_t hs[static restrict 2u],
                uint64_t k1, uint64_t k2) {
#define h1 hs[0u]
#define h2 hs[1u]
    k1 *= SCC_MURMUR64_C1;
    k1 = scc_rol64(k1, 31u);
    k1 *= SCC_MURMUR64_C2;

    h1 ^= k1;
    h1 = scc_rol64(h1, 27u);
    h1 += h2;
    h1 *= 5u;
    h1 += UINT32_C(0x52dce729);

    k2 *= SCC_MURMUR64_C2;
    k2 = scc_rol64(k2, 33u);
    k2 *= SCC_MURMUR64_C1;

    h2 ^= k2;
    h2 = scc_rol64(h2, 31u);
    h2 += h1;
    h2 *= 5u;
    h2 += UINT32_C(0x38495ab5);
#undef h1
#undef h2
}

static inline void scc_murmur64_128_unaligned_main(uint64_t hs[static restrict 2u],
                void const *data, size_t size) {
    unsigned long res = (unsigned long)data & (scc_alignof(uint64_t) - 1u);
    uint64_t tmp = UINT64_C(0);
    memcpy((unsigned char *)&tmp + sizeof(uint64_t) - res, data, res);
    assert(tmp);

    uint64_t const *p64 = (void const *)((unsigned char const *)data + res);

    unsigned const rsft = res << 3u;
    unsigned const msft = (sizeof(tmp) - res) << 3u;

    for (unsigned i = 0u; i < size >> 4u; ++i) {
        uint64_t k2 = p64[i << 1u];
        uint64_t k1 = (tmp >> msft) | (k2 << rsft);
        k2 >>= msft;
        tmp = p64[(i << 1u) + 1u];
        k2 |= (tmp << rsft);
        scc_murmur64_128_main_calc(hs, k1, k2);
    }
}

static inline void scc_murmur64_128_aligned_main(uint64_t hs[static restrict 2u],
                    void const *restrict data, size_t size) {
    uint64_t const *p64 = data;
    for (unsigned i = 0u; i < size >> 4u; ++i) {
        uint64_t k1 = p64[i << 1u];
        uint64_t k2 = p64[(i << 1u) + 1u];
        scc_murmur64_128_main_calc(hs, k1, k2);
    }
}

static inline void scc_murmur64_128_residual(uint64_t hs[static restrict 1u],
                void const *data, size_t size) {
    unsigned char const *p = data;
    p += size & ~15u;

    uint64_t k1 = UINT64_C(0);
    uint64_t k2 = UINT64_C(0);

    switch (size & 15u) {
    case 15u: k2 ^= (uint64_t)p[14u] << 48u;
    case 14u: k2 ^= (uint64_t)p[13u] << 40u;
    case 13u: k2 ^= (uint64_t)p[12u] << 32u;
    case 12u: k2 ^= (uint64_t)p[11u] << 24u;
    case 11u: k2 ^= (uint64_t)p[10u] << 16u;
    case 10u: k2 ^= (uint64_t)p[9u] << 8u;
    case 9u:
        k2 ^= (uint64_t)p[8u];
        k2 *= SCC_MURMUR64_C2;
        k2 = scc_rol64(k2, 33u);
        k2 *= SCC_MURMUR64_C1;
        hs[1u] ^= k2;
        /* fallthrough */
    case 8u: k1 ^= (uint64_t)p[7u] << 56u;
    case 7u: k1 ^= (uint64_t)p[6u] << 48u;
    case 6u: k1 ^= (uint64_t)p[5u] << 40u;
    case 5u: k1 ^= (uint64_t)p[4u] << 32u;
    case 4u: k1 ^= (uint64_t)p[3u] << 24u;
    case 3u: k1 ^= (uint64_t)p[2u] << 16u;
    case 2u: k1 ^= (uint64_t)p[1u] << 8u;
    case 1u:
        k1 ^= (uint64_t)p[0u];
        break;
    }

    k1 *= SCC_MURMUR64_C1;
    k1 = scc_rol64(k1, 31u);
    k1 *= SCC_MURMUR64_C2;
    hs[0u] ^= k1;
}

void scc_murmur64_128(struct scc_digest128 *digest, void const *data,
        size_t size, uint_fast32_t seed) {
    uint64_t hs[] = { seed, seed };

    if ((unsigned long)data & (scc_alignof(uint64_t) - 1u))
        scc_murmur64_128_unaligned_main(hs, data, size);
    else
        scc_murmur64_128_aligned_main(hs, data, size);

    if (size & 15u)
        scc_murmur64_128_residual(hs, data, size);

    hs[0u] ^= size;
    hs[1u] ^= size;

    hs[0u] += hs[1u];
    hs[1u] += hs[0u];

    hs[0u] = scc_fmix64(hs[0u]);
    hs[1u] = scc_fmix64(hs[1u]);

    hs[0u] += hs[1u];
    hs[1u] += hs[0u];

    scc_static_assert(sizeof(digest->digest) == sizeof(hs));
    memcpy(digest->digest, hs, sizeof(hs));
}

#endif /* SCC_HAVE_UINT64_T */
