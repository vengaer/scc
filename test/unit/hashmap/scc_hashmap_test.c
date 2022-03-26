#include <scc/scc_hashmap.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

/* test_scc_hashmap_init
 *
 * Initialize a hash map, verify its size and free it
 */
void test_scc_hashmap_init(void) {
    scc_hashmap(int, unsigned) map = scc_hashmap_init(int, unsigned, eq);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_hashmap_size(map));
    scc_hashmap_free(map);
}

/* test_scc_hashmap_insert_changes_size
 *
 * Repeatedly insert elements in the hash table and
 * verify that its size is increased
 */
void test_scc_hashmap_insert_changes_size(void) {
    enum { TESTSIZE = 128 };
    scc_hashmap(int, unsigned short) map = scc_hashmap_init(int, unsigned short, eq);
    for(int i = 0u; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, (unsigned short)i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashmap_size(map));
    }
    scc_hashmap_free(map);
}
