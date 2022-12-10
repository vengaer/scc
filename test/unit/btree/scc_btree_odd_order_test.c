#include <inspect/btree_inspect.h>
#include <scc/btree.h>
#include <scc/scc_mem.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btree_insert_odd_order(void) {
    enum { TEST_SIZE = 3200 };

    scc_btree(int) btree = scc_btree_with_order(int, compare, 5);

    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

        for(int j = 0; j < i; ++j) {
            TEST_ASSERT_TRUE(scc_btree_find(btree, j));
        }
    }

    scc_btree_free(btree);
}

void test_scc_btree_insert_odd_order_reverse(void) {
    enum { TEST_SIZE = 5000 };

    scc_btree(int) btree = scc_btree_with_order(int, compare, 7);

    for(int i = TEST_SIZE; i > 0; --i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        TEST_ASSERT_EQUAL_UINT64(TEST_SIZE - i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

        for(int j = TEST_SIZE; j > i; --j) {
            TEST_ASSERT_TRUE(scc_btree_find(btree, j));
        }
    }

    scc_btree_free(btree);
}

void test_scc_btree_insert_odd_order_middle_split(void) {

    scc_btree(int) btree = scc_btree_with_order(int, compare, 5);
    for(int i = 0; i < 15; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        for(int j = 0; j < i; ++j) {
            TEST_ASSERT_TRUE(scc_btree_find(btree, j));
        }
    }
    TEST_ASSERT_TRUE(scc_btree_insert(&btree, 16));
    TEST_ASSERT_TRUE(scc_btree_insert(&btree, 17));
    for(int j = 0; j < 15; ++j) {
        TEST_ASSERT_TRUE(scc_btree_find(btree, j));
    }

    TEST_ASSERT_TRUE(scc_btree_find(btree, 16));
    TEST_ASSERT_TRUE(scc_btree_find(btree, 17));

    TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

    TEST_ASSERT_TRUE(scc_btree_insert(&btree, 15));
    TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    scc_btree_free(btree);
}

void test_scc_btree_insert_non_monotonic_odd_order(void) {
    enum { TEST_SIZE = 3200 };
    int data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    scc_btree(int) btree = scc_btree_with_order(int, compare, 5);

    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i % scc_arrsize(data)]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }

    scc_btree_free(btree);
}

void test_scc_btree_remove_odd_order(void) {
    enum { TEST_SIZE = 320 };
    scc_btree(int) btree = scc_btree_with_order(int, compare, 5);

    for(int i = 0;  i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
    }

    int const *p;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_remove(btree, i));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

        for(int j = i + 1; j < TEST_SIZE; ++j) {
            p = scc_btree_find(btree, j);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(j, *p);
        }
    }

    scc_btree_free(btree);
}
