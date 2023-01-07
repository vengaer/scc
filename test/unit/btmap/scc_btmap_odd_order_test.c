#include <inspect/btmap_inspect.h>
#include <scc/btmap.h>
#include <scc/mem.h>

#include <unity.h>

int ocompare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btmap_insert_odd_order(void) {
    enum { TEST_SIZE = 3200 };

    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ocompare, 5);

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

void test_scc_btmap_insert_odd_order_reverse(void) {
    enum { TEST_SIZE = 5000 };

    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ocompare, 7);

    int *p;
    for(int i = TEST_SIZE; i > 0; --i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i + 1));
        TEST_ASSERT_EQUAL_UINT64(TEST_SIZE - i + 1ull, scc_btmap_size(btmap));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

        for(int j = TEST_SIZE; j > i; --j) {
            p = scc_btmap_find(btmap, j);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(j + 1, *p);
        }
    }

    scc_btmap_free(btmap);
}

void test_scc_btmap_insert_odd_order_middle_split(void) {
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ocompare, 5);

    int *p;
    for(int i = 0; i < 15; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i + 1));
        for(int j = 0; j < i; ++j) {
            p = scc_btmap_find(btmap, j);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(j + 1, *p);
        }
    }
    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 16, 17));
    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 17, 18));
    for(int j = 0; j < 15; ++j) {
        p = scc_btmap_find(btmap, j);
        TEST_ASSERT_TRUE(p);
        TEST_ASSERT_EQUAL_INT32(j + 1, *p);
    }

    p = scc_btmap_find(btmap, 16);
    TEST_ASSERT_TRUE(p);
    TEST_ASSERT_EQUAL_INT32(17, *p);
    p = scc_btmap_find(btmap, 17);
    TEST_ASSERT_TRUE(p);
    TEST_ASSERT_EQUAL_INT32(18, *p);

    TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 15, 0));
    p = scc_btmap_find(btmap, 15);
    TEST_ASSERT_TRUE(p);
    TEST_ASSERT_EQUAL_INT32(0, *p);
    TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));
    scc_btmap_free(btmap);
}

void test_scc_btmap_odd_insert_overwrite(void) {
    enum { TEST_SIZE = 3200 };
    int keys[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ocompare, 32);

    size_t expsize;
    int *val;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, keys[i % scc_arrsize(keys)], i));
        expsize = i >= (int)scc_arrsize(keys) ? scc_arrsize(keys) : i + 1ull;
        TEST_ASSERT_EQUAL_UINT64(expsize, scc_btmap_size(btmap));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

        for(int j = i; j > 0 && j > i - (int)scc_arrsize(keys); --j) {
            val = scc_btmap_find(btmap, j % scc_arrsize(keys));
            TEST_ASSERT_TRUE(val);
            TEST_ASSERT_EQUAL_INT32(j, *val);
        }
    }

    scc_btmap_free(btmap);
}

void test_scc_btmap_remove_odd_order(void) {
    enum { TEST_SIZE = 320 };
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ocompare, 5);

    for(int i = 0;  i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i - 1));
    }

    int const *p;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_remove(btmap, i));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

        for(int j = i + 1; j < TEST_SIZE; ++j) {
            p = scc_btmap_find(btmap, j);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(j - 1, *p);
        }
    }

    scc_btmap_free(btmap);
}
