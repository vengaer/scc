#include <scc/scc_rbtree.h>

struct scc_rbtree *scc_rbtree_impl_init(struct scc_rbtree *tree, scc_rbcompare compare);
struct scc_rbnode *scc_rbtree_impl_root(struct scc_rbtree *tree);
