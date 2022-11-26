#include <inspect/scc_btmap_inspect.h>
#include <scc/scc_btmap.h>
#include <scc/scc_mem.h>

#include <string.h>

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

void test_scc_btmap_insert_replace(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, compare);
    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 1, 1));
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_btmap_size(btmap));

    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 2, 3));
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_btmap_size(btmap));

    TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, 1, 3));
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_btmap_size(btmap));

    scc_btmap_free(btmap);
}

void test_scc_btmap_size_empty(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, compare);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_btmap_size(btmap));
    scc_btmap_free(btmap);
}

void test_scc_btmap_insert_default_order(void) {
    enum { TEST_SIZE = 3200 };
    typedef struct { char p[32]; } str;
    int r;
    str s;
    str *sp;
    scc_btmap(int, str) btmap = scc_btmap_new(int, str, compare);
    for(int i = 0; i < TEST_SIZE; ++i) {
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
    enum { TEST_SIZE = 6400 };
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, compare, 500);
    for(int i = TEST_SIZE; i >= 0; --i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, i, i << 1));
        TEST_ASSERT_EQUAL_UINT64((TEST_SIZE - i) + 1ull, scc_btmap_size(btmap));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));
    }
    scc_btmap_free(btmap);
}

void test_scc_btmap_insert_overwrite(void) {
    enum { TEST_SIZE = 3200 };
    int keys[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, compare, 32);

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
