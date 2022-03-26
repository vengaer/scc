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
