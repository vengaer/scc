#include <scc/scc_rbtree.h>

#include <unity.h>

int compare(struct scc_rbnode const *left, struct scc_rbnode const *right) {
    (void)left;
    (void)right;
    return 0;
}

void test_scc_rbtree_init(void) {
    struct scc_rbtree *tree = scc_rbtree_init(compare);
    TEST_ASSERT_EQUAL_UINT64(0, tree->rt_size);
}
