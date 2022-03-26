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
    scc_hashmap(unsigned, unsigned short) map = scc_hashmap_init(unsigned, unsigned short, eq);

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
    scc_hashmap(unsigned, unsigned short) map = scc_hashmap_init(unsigned, unsigned short, eq);

    unsigned short *elem;
    for(unsigned i = 0u; i < scc_arrsize(keys) - 1u; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], vals[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashmap_size(map));
        elem = scc_hashmap_find(map, keys[i]);
        TEST_ASSERT_TRUE(!!elem);
        TEST_ASSERT_EQUAL_UINT16(vals[i], *elem);
    }

    unsigned i = scc_arrsize(keys) - 1u;
    uint16_t *old = scc_hashmap_find(map, keys[i]);
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
