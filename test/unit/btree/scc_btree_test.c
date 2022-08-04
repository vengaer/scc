#include <inspect/scc_btree_inspect.h>
#include <scc/scc_btree.h>
#include <scc/scc_mem.h>

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
    enum { TEST_SIZE = 3200 };
    scc_btree(int) btree = scc_btree_new(int, compare);
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

void test_scc_btree_insert_order_328(void) {
    enum { TEST_SIZE = 6400 };
    scc_btree(int) btree = scc_btree_with_order(int, compare, 328);
    for(int i = TEST_SIZE; i >= 0; --i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        TEST_ASSERT_EQUAL_UINT64((TEST_SIZE - i) + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    scc_btree_free(btree);
}

void test_scc_btree_insert_non_monotonic(void) {
    enum { TEST_SIZE = 3200 };
    int data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    scc_btree(int) btree = scc_btree_with_order(int, compare, 32);

    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i % scc_arrsize(data)]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }

    scc_btree_free(btree);
}

void test_scc_btree_find(void) {
    enum { TEST_SIZE = 3200 };

    scc_btree(int) btree = scc_btree_with_order(int, compare, 88);

    int const *p;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        p = scc_btree_find(btree, i);
        TEST_ASSERT_TRUE(p);
        TEST_ASSERT_EQUAL_INT32(i, *p);

        for(int j = i + 1; j < TEST_SIZE; ++j) {
            TEST_ASSERT_FALSE(scc_btree_find(btree, j));
        }
    }
    scc_btree_free(btree);
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

void test_scc_btree_remove_leaf(void) {
    int leafvals[] = { 0, 1, 3, 4, 5, 6 };

    for(int i = 0; i < (int)scc_arrsize(leafvals); ++i) {
        scc_btree(int) btree = scc_btree_new(int, compare);

        for(int j = 0; j < (int)scc_arrsize(leafvals) + 1; ++j) {
            TEST_ASSERT_TRUE(scc_btree_insert(&btree, j));
        }
        TEST_ASSERT_TRUE(scc_btree_remove(btree, leafvals[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

        int const *p;
        for(int j = 0; j < (int)scc_arrsize(leafvals) + 1; ++j) {
            if(j == leafvals[i]) {
                continue;
            }
            p = scc_btree_find(btree, j);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(j, *p);
        }

        TEST_ASSERT_FALSE(scc_btree_find(btree, leafvals[i]));

        scc_btree_free(btree);
    }
}

void test_scc_btree_remove_root(void) {
    scc_btree(int) btree = scc_btree_new(int, compare);

    for(int i = 0; i < 7; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
    }

    TEST_ASSERT_TRUE(scc_btree_remove(btree, 2));
    TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

    int const *p;
    for(int i = 0; i < 7; ++i) {
        p = scc_btree_find(btree, i);
        if(i == 2) {
            TEST_ASSERT_FALSE(p);
        }
        else {
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(i, *p);
        }
    }

    scc_btree_free(btree);
}
