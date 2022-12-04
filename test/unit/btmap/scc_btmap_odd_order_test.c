#include <inspect/scc_btmap_inspect.h>
#include <scc/scc_btmap.h>
#include <scc/scc_mem.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btmap_insert_odd_order(void) {
    enum { TEST_SIZE = 3200 };

    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, compare, 5);

    int *p;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i << 1));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btmap_size(btmap));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

        for(int j = 0; j < i; ++j) {
            p = scc_btmap_find(btmap, j);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(j << 1, *p);
        }
    }

    scc_btmap_free(btmap);
}
