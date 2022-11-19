#include <inspect/scc_btree_inspect.h>
#include <scc/scc_btmap.h>
#include <scc/scc_mem.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btmap_new(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, compare);
    scc_btmap_free(btmap);
}

void test_scc_btmap_with_order(void) {
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, compare, 4);
    TEST_ASSERT_EQUAL_UINT64(4ull, scc_btmap_order(btmap));
    scc_btmap_free(btmap);

    btmap = scc_btmap_with_order(int, int, compare, 32);
    TEST_ASSERT_EQUAL_UINT64(32ull, scc_btmap_order(btmap));
    scc_btmap_free(btmap);
}
