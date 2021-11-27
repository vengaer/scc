#include <scc/scc_rbtree.h>

#include <unity.h>

int compare(void const *left, void const *right) {
    return *(int const *)right - *(int const *)left;
}

void test_scc_rbtree_init(void) {
    int *tree = scc_rbtree_init(int, compare);
    scc_rbtree_free(tree);
}

void test_scc_rbtree_size(void) {
    int *tree = scc_rbtree_init(int, compare);
    TEST_ASSERT_EQUAL_UINT64(0u, scc_rbtree_size(tree));
    scc_rbtree_free(tree);
}

void test_scc_rbtree_insert(void) {
    int *handle = scc_rbtree_init(int, compare);
    for(int i = 0; i < 3200; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(handle, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_rbtree_size(handle));
        for(int j = 0; j <= i; j++) {
            TEST_ASSERT_FALSE(scc_rbtree_insert(handle, j));
        }
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_find(void) {
    int *handle = scc_rbtree_init(int, compare);
    for(int i = 0; i < 368; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(handle, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_rbtree_size(handle));

        for(int j = 0; j <= i; j++) {
            TEST_ASSERT_TRUE(scc_rbtree_find(handle, j));
        }
    }

    for(int i = 0; i < 368; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_find(handle, i));
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_remove(void) {
    int *handle = scc_rbtree_init(int, compare);
    for(int i = 0; i < 500; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(handle, i));
    }
    for(int i = 0; i < 500; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, i));
        TEST_ASSERT_FALSE(scc_rbtree_remove(handle, i));
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_foreach(void) {
    int *handle = scc_rbtree_init(int, compare);
    for(int i = 0; i < 500; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(handle, i));
    }
    int i = 0;
    int const *iter;
    scc_rbtree_foreach(iter, handle) {
        TEST_ASSERT_EQUAL_INT32(i++, *iter);
    }
    scc_rbtree_free(handle);
}
