#include <inspect/btree_inspect.h>
#include <scc/btree.h>
#include <scc/mem.h>

#include <unity.h>

#ifdef SCC_MUTATION_TEST
enum { OTEST_SIZE = 64 };
#else
enum { OTEST_SIZE = 3200 };
#endif

int ocompare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btree_insert_odd_order(void) {
    scc_btree(int) btree = scc_btree_with_order(int, ocompare, 5);

    for(int i = 0; i < OTEST_SIZE; ++i) {
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
    scc_btree(int) btree = scc_btree_with_order(int, ocompare, 7);

    for(int i = OTEST_SIZE; i > 0; --i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
        TEST_ASSERT_EQUAL_UINT64(OTEST_SIZE - i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

        for(int j = OTEST_SIZE; j > i; --j) {
            TEST_ASSERT_TRUE(scc_btree_find(btree, j));
        }
    }

    scc_btree_free(btree);
}

void test_scc_btree_insert_odd_order_middle_split(void) {

    scc_btree(int) btree = scc_btree_with_order(int, ocompare, 5);
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
    int data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    scc_btree(int) btree = scc_btree_with_order(int, ocompare, 5);

    for(int i = 0; i < OTEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i % scc_arrsize(data)]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btree_size(btree));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }

    scc_btree_free(btree);
}

void test_scc_btree_remove_odd_order(void) {
    scc_btree(int) btree = scc_btree_with_order(int, ocompare, 5);

    for(int i = 0;  i < OTEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
    }

    int const *p;
    for(int i = 0; i < OTEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btree_remove(btree, i));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));

        for(int j = i + 1; j < OTEST_SIZE; ++j) {
            p = scc_btree_find(btree, j);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(j, *p);
        }
    }

    scc_btree_free(btree);
}

void test_scc_btree_clone(void) {
    scc_btree(int) btree = scc_btree_with_order(int, ocompare, 27);

    for(int i = 0; i < 371; ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, i));
    }

    TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    scc_btree(int) nbtree = scc_btree_clone(btree);

    TEST_ASSERT_TRUE(!!nbtree);

    TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(nbtree));

    int const *old;
    int const *new;
    for(int i = 0; i < (int)scc_btree_size(btree); ++i) {
        old = scc_btree_find(btree, i);
        TEST_ASSERT_TRUE(!!old);
        new = scc_btree_find(nbtree, i);
        TEST_ASSERT_TRUE(!!new);
        TEST_ASSERT_EQUAL_INT32(*old, *new);
        TEST_ASSERT_FALSE(old == new);
    }
    TEST_ASSERT_EQUAL_UINT64(scc_btree_size(btree), scc_btree_size(nbtree));

    scc_btree_free(btree);
    scc_btree_free(nbtree);

}
