#include <inspect/btmap_inspect.h>
#include <scc/btmap.h>
#include <scc/mem.h>

#include <string.h>

#include <unity.h>

#ifdef SCC_MUTATION_TEST
enum { ETEST_SIZE = 64 };
#else
enum { ETEST_SIZE = 3200 };
#endif


int ecompare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btmap_new(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, ecompare);
    scc_btmap_free(btmap);
}

void test_scc_btmap_with_order(void) {
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ecompare, 4);
    TEST_ASSERT_EQUAL_UINT64(4ull, scc_btmap_order(btmap));
    scc_btmap_free(btmap);

    btmap = scc_btmap_with_order(int, int, ecompare, 32);
    TEST_ASSERT_EQUAL_UINT64(32ull, scc_btmap_order(btmap));
    scc_btmap_free(btmap);
}

void test_scc_btmap_insert_replace(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, ecompare);
    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 1, 1));
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_btmap_size(btmap));

    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 2, 3));
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_btmap_size(btmap));

    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 1, 3));
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_btmap_size(btmap));

    scc_btmap_free(btmap);
}

void test_scc_btmap_size_empty(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, ecompare);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_btmap_size(btmap));
    scc_btmap_free(btmap);
}

void test_scc_btmap_insert_default_order(void) {
    typedef struct { char p[32]; } str;
    int r;
    str s;
    str *sp;
    scc_btmap(int, str) btmap = scc_btmap_new(int, str, ecompare);
    for(int i = 0; i < ETEST_SIZE; ++i) {
        r = snprintf(s.p, sizeof(s.p), "s%d", i);
        TEST_ASSERT_TRUE(r >= 0 && r < (int)sizeof(s.p));
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, s));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_btmap_size(btmap));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

        for(int j = 0; j < i; ++j) {
            sp = scc_btmap_find(btmap, j);
            TEST_ASSERT_TRUE(sp);
            r = snprintf(s.p, sizeof(s.p), "s%d", j);
            TEST_ASSERT_TRUE(r >= 0 && r < (int)sizeof(s.p));
            TEST_ASSERT_EQUAL_INT32(0, strcmp(sp->p, s.p));
        }
    }

    scc_btmap_free(btmap);
}

void test_scc_btmap_insert_order_500(void) {
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ecompare, 500);
    for(int i = ETEST_SIZE; i >= 0; --i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i << 1));
        TEST_ASSERT_EQUAL_UINT64((ETEST_SIZE - i) + 1ull, scc_btmap_size(btmap));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));
    }
    scc_btmap_free(btmap);
}

void test_scc_btmap_even_insert_overwrite(void) {
    int keys[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ecompare, 32);

    size_t expsize;
    int *val;
    for(int i = 0; i < ETEST_SIZE; ++i) {
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

void test_scc_btmap_find(void) {
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, ecompare, 88);

    int const *p;
    for(int i = 0; i < ETEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i << 1));
        p = scc_btmap_find(btmap, i);
        TEST_ASSERT_TRUE(p);
        TEST_ASSERT_EQUAL_INT32(i << 1, *p);

        for(int j = i + 1; j < ETEST_SIZE; ++j) {
            TEST_ASSERT_FALSE(scc_btmap_find(btmap, j));
        }
    }
    scc_btmap_free(btmap);
}

void test_scc_btmap_remove_leaf(void) {
    int leafvals[] = { 0, 1, 3, 4, 5, 6 };

    for(int i = 0; i < (int)scc_arrsize(leafvals); ++i) {
        scc_btmap(int, int) btmap = scc_btmap_new(int, int, ecompare);

        /* Insert 0, 1, 2, 3, 4, 5, 6 */
        for(int j = 0; j < 7; ++j) {
            TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, j, j));
        }
        TEST_ASSERT_TRUE(scc_btmap_remove(btmap, leafvals[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

        int const *p;
        for(int j = 0; j < 7; ++j) {
            p = scc_btmap_find(btmap, j);
            if(j == leafvals[i]) {
                TEST_ASSERT_FALSE(!!p);
            }
            else {
                TEST_ASSERT_TRUE(!!p);
                TEST_ASSERT_EQUAL_INT32(j, *p);
            }
        }

        TEST_ASSERT_FALSE(scc_btmap_find(btmap, leafvals[i]));

        scc_btmap_free(btmap);
    }
}

void test_scc_btmap_remove_root(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, ecompare);

    for(int i = 0; i < 7; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i));
    }

    TEST_ASSERT_TRUE(scc_btmap_remove(btmap, 2));
    TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

    int const *p;
    for(int i = 0; i < 7; ++i) {
        p = scc_btmap_find(btmap, i);
        if(i == 2) {
            TEST_ASSERT_FALSE(p);
        }
        else {
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(i, *p);
        }
    }

    scc_btmap_free(btmap);
}

void test_scc_btmap_insert_descending_order(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, ecompare);

    for(int i = 32; i; --i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i));
    }

    int const *val;
    for(int i = 1; i < 33; ++i) {
        val = scc_btmap_find(btmap, i);
        TEST_ASSERT_TRUE(!!val);
        TEST_ASSERT_EQUAL_INT32(i, *val);
    }

    scc_btmap_free(btmap);
}
