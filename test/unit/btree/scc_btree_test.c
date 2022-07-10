#include <scc/scc_btree.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btree_new(void) {
    scc_btree(int) btree = scc_btree_new(int, compare);
    scc_btree_free(btree);
}

void test_scc_btree_with_order(void) {
    scc_btree(int) btree = scc_btree_with_order(int, compare, 2);
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_btree_order(btree));
    scc_btree_free(btree);

    btree = scc_btree_with_order(int, compare, 32);
    TEST_ASSERT_EQUAL_UINT64(32ull, scc_btree_order(btree));
    scc_btree_free(btree);
}

void test_scc_btree_size_empty(void) {
    scc_btree(int) btree = scc_btree_new(int, compare);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_btree_size(btree));
    scc_btree_free(btree);
}
