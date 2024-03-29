#include <inspect/btree_inspect.h>
#include <scc/btree.h>
#include <scc/mem.h>

#include <unity.h>

#ifdef SCC_MUTATION_TEST
enum { ETEST_SIZE = 64 };
#else
enum { ETEST_SIZE = 3200 };
#endif

int ecompare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btree_new(void) {
    scc_btree(int) btree = scc_btree_new(int, ecompare);
    struct scc_btree_base *base = scc_btree_impl_base(btree);
    TEST_ASSERT_FALSE(base->bt_dynalloc);
    scc_btree_free(btree);
}

void test_scc_btree_with_order(void) {
    scc_btree(int) btree = scc_btree_with_order(int, ecompare, 4);
    TEST_ASSERT_EQUAL_UINT64(4ull, scc_btree_order(btree));
    scc_btree_free(btree);

    btree = scc_btree_with_order(int, ecompare, 32);
    TEST_ASSERT_EQUAL_UINT64(32ull, scc_btree_order(btree));
    scc_btree_free(btree);
}

void test_scc_btree_new_dyn(void) {
    scc_btree(int) btree = scc_btree_new_dyn(int, ecompare);
    TEST_ASSERT_TRUE(!!btree);
    struct scc_btree_base *base = scc_btree_impl_base(btree);
    TEST_ASSERT_TRUE(base->bt_dynalloc);
    scc_btree_free(btree);
}

void test_scc_btree_with_order_dyn(void) {
    scc_btree(int) btree = scc_btree_with_order_dyn(int, ecompare, 22u);
    TEST_ASSERT_TRUE(!!btree);
    struct scc_btree_base *base = scc_btree_impl_base(btree);
    TEST_ASSERT_TRUE(base->bt_dynalloc);
    TEST_ASSERT_EQUAL_UINT64(22ull, scc_btree_order(btree));
    scc_btree_free(btree);
}

void test_scc_btree_size_empty(void) {
    scc_btree(int) btree = scc_btree_new(int, ecompare);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_btree_size(btree));
    scc_btree_free(btree);
}

void test_scc_btree_insert_default_order(void) {
    scc_btree(int) btree = scc_btree_new(int, ecompare);
    for(int i = 0; i < ETEST_SIZE; ++i) {
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
    scc_btree(int) btree = scc_btree_with_order(int, ecompare, 328);
    for(int i = ETEST_SIZE; i >= 0; --i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        TEST_ASSERT_EQUAL_UINT64((ETEST_SIZE - i) + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    scc_btree_free(btree);
}

void test_scc_btree_insert_non_monotonic(void) {
    int data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    scc_btree(int) btree = scc_btree_with_order(int, ecompare, 32);

    for(int i = 0; i < ETEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i % scc_arrsize(data)]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }

    scc_btree_free(btree);
}

void test_scc_btree_find(void) {
    scc_btree(int) btree = scc_btree_with_order(int, ecompare, 88);

    int const *p;
    for(int i = 0; i < ETEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        p = scc_btree_find(btree, i);
        TEST_ASSERT_TRUE(p);
        TEST_ASSERT_EQUAL_INT32(i, *p);

        for(int j = i + 1; j < ETEST_SIZE; ++j) {
            TEST_ASSERT_FALSE(scc_btree_find(btree, j));
        }
    }
    scc_btree_free(btree);
}

void test_scc_btree_remove_leaf(void) {
    int leafvals[] = { 0, 1, 3, 4, 5, 6 };

    for(int i = 0; i < (int)scc_arrsize(leafvals); ++i) {
        scc_btree(int) btree = scc_btree_new(int, ecompare);

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
    scc_btree(int) btree = scc_btree_new(int, ecompare);

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
