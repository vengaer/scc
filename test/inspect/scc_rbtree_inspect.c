#include "scc_rbtree_inspect.h"

#include <scc/scc_mem.h>

#include <stdbool.h>
#include <stdio.h>

#define rn_left     rn_bare.un_link.node.left
#define rn_right    rn_bare.un_link.node.right
#define rn_tree     rn_bare.un_link.tree
#define rn_color    rn_bare.color
#define rn_flags    rn_bare.flags

#define rb_root     rb_sentinel.un_link.root

enum { SCC_RBTREE_INSPECT_LEFT = 0 };
enum { SCC_RBTREE_INSPECT_RIGHT = 1 };
enum { SCC_RBTREE_INSPECT_LTHRD = 0x1 };
enum { SCC_RBTREE_INSPECT_RTHRD = 0x2 };
enum { SCC_RBTREE_INSPECT_LEAF = 0x3 };

#define scc_rbnode_value_qual(tree, node, qual)     \
    ((unsigned char qual *)(node) + (tree)->rb_baseoff)

#define scc_rbnode_value(tree, node)                \
    scc_rbnode_value_qual(tree, node,)

static bool scc_rbtree_inspect_thread(struct scc_rbnode const *node, unsigned dir);
static bool scc_rbtree_inspect_red(struct scc_rbnode const *node);
static bool scc_rbtree_inspect_black(struct scc_rbnode const *node);
static bool scc_rbtree_inspect_red_safe(struct scc_rbnode const *node, unsigned dir);
static bool scc_rbtree_inspect_has_red_child(struct scc_rbnode const *node);
static bool scc_rbtree_inspect_red_violation(struct scc_rbnode const *node);
static bool scc_rbtree_inspect_has_child(struct scc_rbnode const *node, unsigned dir);
static bool scc_rbtree_inspect_leaf(struct scc_rbnode const *node);
static int scc_rbtree_inspect_compare(
    struct scc_rbtree const *restrict tree,
    struct scc_rbnode const *restrict left,
    struct scc_rbnode const *restrict right
);
static bool scc_rbtree_inspect_left_violation(
    struct scc_rbtree const *tree,
    struct scc_rbnode const *node
);
static inline bool scc_rbtree_inspect_right_violation(
    struct scc_rbtree const *tree,
    struct scc_rbnode const *node
);
static unsigned long long scc_rbtree_inspect_properties_impl(
    struct scc_rbtree const *tree,
    struct scc_rbnode const *node
);

static inline bool scc_rbtree_inspect_thread(struct scc_rbnode const *node, unsigned dir) {
    return node->rn_flags & (1 << dir);
}

static inline bool scc_rbtree_inspect_red(struct scc_rbnode const *node) {
    return node->rn_color == scc_rbcolor_red;
}

static inline bool scc_rbtree_inspect_black(struct scc_rbnode const *node) {
    return !scc_rbtree_inspect_red(node);
}

static inline bool scc_rbtree_inspect_red_safe(struct scc_rbnode const *node, unsigned dir) {
    return !scc_rbtree_inspect_thread(node, dir) &&
            scc_rbtree_inspect_red(dir ? node->rn_left : node->rn_right);
}

static inline bool scc_rbtree_inspect_has_red_child(struct scc_rbnode const *node) {
    return scc_rbtree_inspect_red_safe(node, SCC_RBTREE_INSPECT_LEFT) ||
           scc_rbtree_inspect_red_safe(node, SCC_RBTREE_INSPECT_RIGHT);
}

static inline bool scc_rbtree_inspect_red_violation(struct scc_rbnode const *node) {
    return  scc_rbtree_inspect_red(node) && scc_rbtree_inspect_has_red_child(node);
}

static inline bool scc_rbtree_inspect_has_child(struct scc_rbnode const *node, unsigned dir) {
    return !(node->rn_flags & (1 << dir));
}

static inline bool scc_rbtree_inspect_leaf(struct scc_rbnode const *node) {
    return (node->rn_flags & SCC_RBTREE_INSPECT_LEAF) == SCC_RBTREE_INSPECT_LEAF;
}

static inline int scc_rbtree_inspect_compare(
    struct scc_rbtree const *restrict tree,
    struct scc_rbnode const *restrict left,
    struct scc_rbnode const *restrict right
) {
    void const *laddr = scc_rbnode_value_qual(tree, left, const);
    void const *raddr = scc_rbnode_value_qual(tree, right, const);
    return tree->rb_compare(laddr, raddr);
}

static inline bool scc_rbtree_inspect_left_violation(
    struct scc_rbtree const *tree,
    struct scc_rbnode const *node
) {
    return scc_rbtree_inspect_has_child(node, SCC_RBTREE_INSPECT_LEFT) &&
           scc_rbtree_inspect_compare(tree, node->rn_left, node) >= 0;
}

static inline bool scc_rbtree_inspect_right_violation(
    struct scc_rbtree const *tree,
    struct scc_rbnode const *node
) {
    return scc_rbtree_inspect_has_child(node, SCC_RBTREE_INSPECT_RIGHT) &&
           scc_rbtree_inspect_compare(tree, node, node->rn_right) >= 0;
}

static unsigned long long scc_rbtree_inspect_properties_impl(
    struct scc_rbtree const *tree,
    struct scc_rbnode const *node
) {
    if(node->rn_left == node) {
        /* Left link causes loop */
        return SCC_RBTREE_ERR_LOOP;
    }
    if(node->rn_right == node) {
        /* Right link causes loop */
        return SCC_RBTREE_ERR_LOOP;
    }

    if(scc_rbtree_inspect_red_violation(node)) {
        /* Red node has 1 or more red children */
        return SCC_RBTREE_ERR_RED;
    }
    if(scc_rbtree_inspect_left_violation(tree, node)) {
        /* Left child greater or equal to node */
        return SCC_RBTREE_ERR_LEFT;
    }
    if(scc_rbtree_inspect_right_violation(tree, node)) {
        /* Right child smaller than or equal to node */
        return SCC_RBTREE_ERR_RIGHT;
    }

    unsigned black_height = scc_rbtree_inspect_black(node);
    if(!node->rn_flags) {
        unsigned long long lsubtree = scc_rbtree_inspect_properties_impl(tree, node->rn_left);
        unsigned long long rsubtree = scc_rbtree_inspect_properties_impl(tree, node->rn_right);
        unsigned long long subtrees = lsubtree | rsubtree;
        if(subtrees & SCC_RBTREE_ERR_MASK) {
            /* Some violation in subtree */
            return subtrees;
        }

        if(lsubtree != rsubtree) {
            /* Black heights differ between left and right subtrees */
            return SCC_RBTREE_ERR_BLACK;
        }

        return black_height + rsubtree;
    }
    if(!scc_rbtree_inspect_leaf(node)) {
        struct scc_rbnode *n;
        if(node->rn_flags & SCC_RBTREE_INSPECT_RTHRD) {
            n = node->rn_left;
        }
        else {
            n = node->rn_right;
        }

        unsigned long long height = scc_rbtree_inspect_properties_impl(tree, n);
        if(height & SCC_RBTREE_ERR_MASK) {
            /* Error in subtree */
            return height;
        }
        if(height) {
            /* Must be no black nodes in child of node with only one subtree */
            return SCC_RBTREE_ERR_BLACK;
        }
    }

    return black_height;
}


unsigned long long scc_rbtree_inspect_properties(void const *handle) {
    struct scc_rbtree const *tree =
        scc_container_qual(
            handle - ((unsigned char const *)handle)[-1],
            struct scc_rbnode,
            rn_buffer,
            const
        )->rn_tree;

    if(!tree->rb_size) {
        return 0;
    }

    if(!scc_rbtree_inspect_black(tree->rb_root)) {
        /* Root must be black */
        return SCC_RBTREE_ERR_ROOT;
    }

    return scc_rbtree_inspect_properties_impl(tree, tree->rb_root);
}

void scc_rbtree_inspect_dump_flags(unsigned long long flags) {
    if(!(flags & SCC_RBTREE_ERR_MASK)) {
        return;
    }
    if(flags & SCC_RBTREE_ERR_RED) {
        puts("Red violation");
    }
    if(flags & SCC_RBTREE_ERR_BLACK) {
        puts("Black height violation");
    }
    if(flags & SCC_RBTREE_ERR_LEFT) {
        puts("Left ordering violation");
    }
    if(flags & SCC_RBTREE_ERR_RIGHT) {
        puts("Right ordering violation");
    }
    if(flags & SCC_RBTREE_ERR_ROOT) {
        puts("Root not black");
    }
    if(flags & SCC_RBTREE_ERR_LOOP) {
        puts("Loop in tree");
    }
}
