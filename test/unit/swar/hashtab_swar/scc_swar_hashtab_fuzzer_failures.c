#include <scc/hashtab.h>
#include <scc/mem.h>

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include <unity.h>

static bool ueq(void const *left, void const *right) {
    return *(uint32_t const *)left == *(uint32_t const *)right;
}

static void run_fuzzer_test(uint32_t const *data, size_t n) {
#ifdef SCC_SIMD_ISA
    extern int scc_simd_support;
    int simdbak = scc_simd_support;
    scc_simd_support = 0;
#endif

    scc_hashtab(uint32_t) tab = scc_hashtab_new(uint32_t, ueq);

    for(unsigned i = 0u; i < n; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, data[i]));
    }

    uint32_t const *elem;
    for(unsigned i = 0u; i < n; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashtab_size(tab));

        for(unsigned j = 0u; j < i; ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
        }
    }
    for(unsigned i = 0u; i < n; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(n - i - 1ull, scc_hashtab_size(tab));

        for(unsigned j = i + 1u; j < n; ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
        }
    }

    TEST_ASSERT_EQUAL_UINT64(0ull, scc_hashtab_size(tab));
    scc_hashtab_free(tab);
#ifdef SCC_SIMD_ISA
    scc_simd_support = simdbak;
#endif
}

void test_scc_swar_hashtab_fuzzer_failure0(void) {
    static uint32_t const vals[] = {
        0xafaffafa, 0xccafafaf, 0x8bcccccc, 0xd0afabcc,
        0x3afafad0, 0x2f0afafa, 0x29af0dca, 0x500007e7,
        0xda000061, 0xdfafafa,  0xabafafaf, 0xfad0d0af,
        0xfafa3afa, 0xdca2f0a,  0xafafafaf, 0xfafaafaf,
        0xfafafac5, 0x80010dfa, 0xd00d373b, 0xd0d0d03b,
        0xfafad0d8, 0x3afafafe, 0xd0dfafa,  0x1afafaf,
        0xf2c8c480, 0x3b80012f, 0xd0d00d33, 0xd0d8d0d0,
        0xfafafafa, 0xaeafaffa, 0xccccafaf, 0xabcccccc,
        0x7e729af,  0x615000,   0xfafafa00, 0xafaf0dfa,
        0xd0afabaf, 0xafaf0dca, 0xfac5fafa, 0x343b8001,
        0xd03bd00d, 0xafaf0d0d, 0xc48001af, 0x12ff2c8,
        0xd373b80,  0xd0d0d0d0, 0x3afafafa, 0x7f000060,
        0xc8c48000, 0x2f2f2ff2, 0x1d0d031,  0x43432a80
    };

    run_fuzzer_test(vals, scc_arrsize(vals));
}

void test_scc_swar_hashtab_fuzzer_failure1(void) {
    static uint32_t vals[] = {
        0xff0aff0a, 0x8e0382ff, 0x2dc0fd72, 0xffffff2e,
        0xffffffff, 0xb198ffff, 0x32360e0e, 0xb1000000,
        0xb1b1b1b1, 0xe3e3e3b1, 0x000000e3, 0x60000000,
        0xffe3e3e3, 0x0e0ee1ff, 0x290a0a0e, 0x0e0eb129,
        0x999999b1, 0x99993f99, 0x9999999d, 0x99996367,
        0xffffff99, 0xfafafa04, 0xff2fffff, 0x002dc0fd,
        0x82ff606d, 0xfd728e03, 0x002e2dc0, 0x0097606d,
        0x0099fa00, 0xfafafafa, 0xdbdb3afa, 0x990dcadb,
        0xdd999999, 0xe1dddddd, 0xe1e1e1e1, 0xdddddddd,
        0xfc7effdd, 0xec8ef5c6, 0xd0d0d00e, 0x2a8001d0
    };

    run_fuzzer_test(vals, scc_arrsize(vals));
}
