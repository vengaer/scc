#include <scc/scc_btree.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btree_init(void) {
    scc_btree(int) btree = scc_btree_init(int, compare);
    scc_btree_free(btree);
}

void test_scc_btree_init_with_order(void) {
    scc_btree(int) btree = scc_btree_init_with_order(int, compare, 2);
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_btree_order(btree));
    scc_btree_free(btree);

    btree = scc_btree_init_with_order(int, compare, 32);
    TEST_ASSERT_EQUAL_UINT64(32ull, scc_btree_order(btree));
    scc_btree_free(btree);
}
