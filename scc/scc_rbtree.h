#ifndef SCC_RBTREE_H
#define SCC_RBTREE_H

#include <stddef.h>

enum { SCC_RBTHRD_LEFT = 0x01 };
enum { SCC_RBTHRD_RIGHT = 0x02 };
enum { SCC_RBTHRD_LEAF = SCC_RBTHRD_LEFT | SCC_RBTHRD_RIGHT };

enum scc_rbdir {
    scc_rbdir_left,
    scc_rbdir_right
};

enum scc_rbcolor {
    scc_rbcolor_black,
    scc_rbcolor_red
};

struct scc_rbnode;

typedef unsigned char scc_rbnode_flags;
typedef int(*scc_rbcompare)(struct scc_rbnode const *, struct scc_rbnode const *);

struct scc_rbnode {
    struct scc_rbnode *rn_left;
    struct scc_rbnode *rn_right;
    enum scc_rbcolor rn_color;
    scc_rbnode_flags rn_flags;
};

struct scc_rbtree {
    size_t rt_size;
    struct scc_rbnode rt_sentinel;
    scc_rbcompare rt_compare;
};

inline struct scc_rbtree *scc_rbtree_impl_init(struct scc_rbtree *tree, scc_rbcompare compare) {
    tree->rt_sentinel.rn_flags = SCC_RBTHRD_LEAF;
    tree->rt_compare = compare;
    return tree;
}

#define scc_rbtree_impl_root(tree)                              \
    ((tree)->rt_sentinel.rn_left)

inline _Bool scc_rbtree_empty(struct scc_rbtree const *tree) {
    return !tree->rt_size;
}

#define scc_rbtree_init(compare)                                \
    scc_rbtree_impl_init(&(struct scc_rbtree){ 0 }, compare)

_Bool scc_rbtree_insert(struct scc_rbtree *restrict tree, struct scc_rbnode *restrict node);


#endif /* SCC_RBTREE_H */
