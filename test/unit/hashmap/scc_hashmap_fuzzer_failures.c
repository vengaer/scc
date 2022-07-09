#include <scc/scc_hashmap.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(unsigned const *)left == *(unsigned const *)right;
}

/* test_scc_hashmap_fuzzer_failure0
 *
 * First failure case detected by fuzzer
 */
void test_scc_hashmap_fuzzer_failure0(void) {
    static unsigned const keys[] = {
        0u, 0u
    };
    static unsigned short const vals[] = {
        0u, 0u
    };
    scc_hashmap(unsigned, unsigned short) map = scc_hashmap_new(unsigned, unsigned short, eq);

    unsigned short *elem;
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[0], vals[0]));
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_hashmap_size(map));
    elem = scc_hashmap_find(map, keys[0]);
    TEST_ASSERT_TRUE(!!elem);
    TEST_ASSERT_EQUAL_UINT16(vals[0], *elem);

    unsigned short *old = scc_hashmap_find(map, keys[1]);
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[1], vals[1]));
    elem = scc_hashmap_find(map, keys[1]);
    TEST_ASSERT_TRUE(!!elem);
    TEST_ASSERT_EQUAL_PTR(old, elem);
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_hashmap_size(map));

    elem = scc_hashmap_find(map, keys[0]);
    TEST_ASSERT_TRUE(!!elem);

    scc_hashmap_free(map);
}

/* test_scc_hashmap_fuzzer_failure1
 *
 * Second failure case detected by fuzzer
 */
void test_scc_hashmap_fuzzer_failure1(void) {
    static unsigned const keys[] = {
        4294967040, 4294967295, 4282253311, 47360,
        10, 4294967295,
    };
    static unsigned short const vals[] = {
        65280, 65535, 65535, 65535,
        65535, 65535
    };
    scc_hashmap(unsigned, unsigned short) map = scc_hashmap_new(unsigned, unsigned short, eq);

    unsigned short *elem;
    for(unsigned i = 0u; i < scc_arrsize(keys) - 1u; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], vals[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashmap_size(map));
        elem = scc_hashmap_find(map, keys[i]);
        TEST_ASSERT_TRUE(!!elem);
        TEST_ASSERT_EQUAL_UINT16(vals[i], *elem);
    }

    unsigned i = scc_arrsize(keys) - 1u;
    unsigned short *old = scc_hashmap_find(map, keys[i]);
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], vals[i]));
    elem = scc_hashmap_find(map, keys[i]);
    TEST_ASSERT_TRUE(!!elem);
    TEST_ASSERT_EQUAL_PTR(old, elem);
    TEST_ASSERT_EQUAL_UINT64(scc_arrsize(keys) - 1ull, scc_hashmap_size(map));
    for(unsigned j = 0u; j < i; ++j) {
        elem = scc_hashmap_find(map, keys[j]);
        TEST_ASSERT_TRUE(!!elem);
    }

    scc_hashmap_free(map);
}

/* test_scc_hashmap_fuzzer_failure2
 *
 * Third failure case detected by fuzzer
 */
void test_scc_hashmap_fuzzer_failure2(void) {
    static unsigned const keys[] = {
        0, 9043968, 512, 11,
    };
    static unsigned const dkeys[] = {
        512, 11
    };
    static unsigned short const vals[] = {
        0, 0, 0, 0
    };
    static unsigned short const dvals[] = {
        0, 138
    };
    scc_hashmap(unsigned, unsigned short) map = scc_hashmap_new(unsigned, unsigned short, eq);

    unsigned short *elem;
    for(unsigned i = 0u; i < scc_arrsize(keys); ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], vals[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashmap_size(map));
        elem = scc_hashmap_find(map, keys[i]);
        TEST_ASSERT_TRUE(!!elem);
        TEST_ASSERT_EQUAL_UINT16(vals[i], *elem);
    }

    unsigned short *old;
    for(unsigned i = 0u; i < scc_arrsize(dkeys); ++i) {
        old = scc_hashmap_find(map, dkeys[i]);
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, dkeys[i], dvals[i]));
        elem = scc_hashmap_find(map, dkeys[i]);
        TEST_ASSERT_TRUE(!!elem);
        TEST_ASSERT_EQUAL_PTR(old, elem);
        TEST_ASSERT_EQUAL_UINT64(scc_arrsize(keys), scc_hashmap_size(map));
        for(unsigned j = 0u; j < scc_arrsize(keys); ++j) {
            elem = scc_hashmap_find(map, keys[j]);
            TEST_ASSERT_TRUE(!!elem);
        }
    }

    scc_hashmap_free(map);
}
/* test_scc_hashmap_fuzzer_failure3
 *
 * Fourth failure case detected by fuzzer
 */
void test_scc_hashmap_fuzzer_failure3(void) {
    static unsigned const keys[] = {
        0,          4194369,    38400,      4294966272,
        3774873599, 1090519040, 16384,      150,
        4278190076, 14745599,   1006895360, 47266876,
        3942786,    1027357696, 1010576444, 976567333,
        252460096,  1016267324, 1010581564, 1090491138,
        4259584,    455224585,  570425344,  8738,
        37632,      16777216,   152895488,  19263724,
        16122080,   2816,       1044250628, 1044266558,
        1010581054, 1010580540, 1009531964, 2466397199,
        16777276,   1010580484, 696386257,  60,
        2265910272, 1010602754, 37502016,   4259731,
        151011583,  1778221,    572653568,  34,
        147,        65536,      3960020224, 3758171632,
        62976,      67108875,   1014774784, 255601724
    };
    static unsigned const dkeys[] = {
        3758171632, 62976,     67108875,   1014774784,
        1010580540, 255601724, 1016267324, 1010581564
    };
    static unsigned short const vals[] = {
        0,     0,     65,    64,
        38400, 0,     64512, 65535,
        65535, 57599, 0,     16640,
        16384, 0,     150,   0,
        65532, 65279, 65535, 224,
        256,   15364, 15420, 721,
        10626, 60,    0,     0,
        15360, 15676, 11324, 15420,
        15397, 14901, 15424, 3852,
        572,   15507, 16444, 15420,
        37634, 16639, 65280, 64,
        11529, 6946,  0,     0,
        0,     8704,  8738,  0,
        37632, 0, 0,  256,
    };
    static unsigned short const dvals[] = {
        0, 0, 0, 2333,
        61676, 293, 224, 246
    };
    scc_hashmap(unsigned, unsigned short) map = scc_hashmap_new(unsigned, unsigned short, eq);

    unsigned short *elem;
    for(unsigned i = 0u; i < scc_arrsize(keys); ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], vals[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashmap_size(map));
        elem = scc_hashmap_find(map, keys[i]);
        TEST_ASSERT_TRUE(!!elem);
        TEST_ASSERT_EQUAL_UINT16(vals[i], *elem);
    }

    unsigned short *old;
    size_t cap;
    for(unsigned i = 0u; i < scc_arrsize(dkeys); ++i) {
        old = scc_hashmap_find(map, dkeys[i]);
        cap = scc_hashmap_capacity(map);
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, dkeys[i], dvals[i]));
        elem = scc_hashmap_find(map, dkeys[i]);
        TEST_ASSERT_TRUE(!!elem);
        if(cap == scc_hashmap_capacity(map)) {
            TEST_ASSERT_EQUAL_PTR(old, elem);
        }
        TEST_ASSERT_EQUAL_UINT64(scc_arrsize(keys), scc_hashmap_size(map));
        for(unsigned j = 0u; j < scc_arrsize(keys); ++j) {
            elem = scc_hashmap_find(map, keys[j]);
            TEST_ASSERT_TRUE(!!elem);
        }
    }

    scc_hashmap_free(map);
}
