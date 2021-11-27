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
