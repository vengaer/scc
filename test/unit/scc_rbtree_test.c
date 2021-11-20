#include <scc/scc_rbtree.h>

#include <unity.h>

void test_scc_rbtree_init(void) {
    struct scc_rbtree *tree = scc_rbtree_init();
    TEST_ASSERT_EQUAL_UINT64(0, tree->size);
}
