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

typedef unsigned char scc_rbnode_flags;

struct scc_rbnode {
    struct scc_rbnode *sc_left;
    struct scc_rbnode *sc_right;
    enum scc_rbcolor sc_color;
    scc_rbnode_flags sc_flags;
};

struct scc_rbtree {
    struct scc_rbnode sentinel;
    size_t size;
};

inline struct scc_rbtree *scc_rbtree_impl_init(struct scc_rbtree *tree) {
    tree->sentinel.sc_flags = SCC_RBTHRD_LEAF;
    return tree;
}

#define scc_rbtree_init()   \
    scc_rbtree_impl_init(&(struct scc_rbtree){ 0 })


#endif /* SCC_RBTREE_H */
