#include <inspect/scc_btree_inspect.h>
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
    scc_btree(int) btree = scc_btree_with_order(int, compare, 4);
    TEST_ASSERT_EQUAL_UINT64(4ull, scc_btree_order(btree));
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

void test_scc_btree_insert_default_order(void) {
    enum { TEST_SIZE = 32000 };
    scc_btree(int) btree = scc_btree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }

    scc_btree_free(btree);
}

void test_scc_btree_insert_order_328(void) {
    enum { TEST_SIZE = 64000 };
    scc_btree(int) btree = scc_btree_with_order(int, compare, 328);
    for(int i = TEST_SIZE; i >= 0; --i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        TEST_ASSERT_EQUAL_UINT64((TEST_SIZE - i) + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    scc_btree_free(btree);
}
