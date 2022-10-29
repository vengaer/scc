#include <inspect/scc_rbtree_inspect.h>
#include <scc/scc_mem.h>
#include <scc/scc_rbmap.h>

#include <unity.h>

static int compare(void const *left, void const *right) {
    return *(int const *)left - *(int const *)right;
}

void test_scc_rbmap_insert(void) {
    enum { TEST_SIZE = 2400 };

    scc_inspect_mask mask;
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_rbmap_size(rbmap));
        for(int j = 0; j <= i; j++) {
            TEST_ASSERT_FALSE(scc_rbmap_insert(&rbmap, j, j));
        }
        mask = scc_rbtree_inspect_properties(rbmap);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_find(void) {
    enum { TEST_SIZE = 1200 };
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);

    scc_inspect_mask mask;
    int *val;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, i));
        val = scc_rbmap_find(rbmap, i);
        TEST_ASSERT_TRUE(!!val);
        TEST_ASSERT_EQUAL_INT32(i, *val);
        *val = i << 1;

        for(int j = 0; j < i; ++j) {
            val = scc_rbmap_find(rbmap, j);
            TEST_ASSERT_EQUAL_INT32(2 * j, *val);
        }
        mask = scc_rbtree_inspect_properties(rbmap);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_remove(void) {
    enum { TEST_SIZE = 5000 };
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, 0));
    }

    scc_inspect_mask mask;
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbmap_remove(rbmap, i));
        TEST_ASSERT_FALSE(scc_rbmap_remove(rbmap, i));
        mask = scc_rbtree_inspect_properties(rbmap);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbmap_free(rbmap);
}
