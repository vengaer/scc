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
    struct scc_rbnode *sc_left;
    struct scc_rbnode *sc_right;
    enum scc_rbcolor sc_color;
    scc_rbnode_flags sc_flags;
};

struct scc_rbtree {
    size_t size;
    struct scc_rbnode sentinel;
    scc_rbcompare sc_compare;
};

inline struct scc_rbtree *scc_rbtree_impl_init(struct scc_rbtree *tree, scc_rbcompare compare) {
    tree->sentinel.sc_flags = SCC_RBTHRD_LEAF;
    tree->sc_compare = compare;
    return tree;
}

inline struct scc_rbnode *scc_rbtree_impl_root(struct scc_rbtree *tree) {
    return tree->sentinel.sc_left;
}

#define scc_rbtree_init(compare)                                \
    scc_rbtree_impl_init(&(struct scc_rbtree){ 0 }, compare)



#endif /* SCC_RBTREE_H */
