#include "rbtree_inspect.h"

#include <scc/mem.h>
#include <scc/stack.h>

#include <stdbool.h>
#include <stdio.h>

enum { SCC_RBTREE_INSPECT_LEFT = 0 };
enum { SCC_RBTREE_INSPECT_RIGHT = 1 };
enum { SCC_RBTREE_INSPECT_LTHRD = 0x1 };
enum { SCC_RBTREE_INSPECT_RTHRD = 0x2 };
enum { SCC_RBTREE_INSPECT_LEAF = 0x3 };

#define rb_root rb_sentinel.rs_left

static inline bool scc_rbtree_inspect_thread(struct scc_rbnode_base const *node, unsigned dir) {
    return node->rn_flags & (1 << dir);
}

static inline bool scc_rbtree_inspect_red(struct scc_rbnode_base const *node) {
    return node->rn_color == scc_rbcolor_red;
}

static inline bool scc_rbtree_inspect_black(struct scc_rbnode_base const *node) {
    return !scc_rbtree_inspect_red(node);
}

static inline bool scc_rbtree_inspect_red_safe(struct scc_rbnode_base const *node, unsigned dir) {
    return !scc_rbtree_inspect_thread(node, dir) &&
            scc_rbtree_inspect_red(dir ? node->rn_left : node->rn_right);
}

static inline bool scc_rbtree_inspect_has_red_child(struct scc_rbnode_base const *node) {
    return scc_rbtree_inspect_red_safe(node, SCC_RBTREE_INSPECT_LEFT) ||
           scc_rbtree_inspect_red_safe(node, SCC_RBTREE_INSPECT_RIGHT);
}

static inline bool scc_rbtree_inspect_red_violation(struct scc_rbnode_base const *node) {
    return  scc_rbtree_inspect_red(node) && scc_rbtree_inspect_has_red_child(node);
}

static inline bool scc_rbtree_inspect_has_child(struct scc_rbnode_base const *node, unsigned dir) {
    return !(node->rn_flags & (1 << dir));
}

static inline int scc_rbtree_inspect_compare(
    struct scc_rbtree_base const *restrict tree,
    struct scc_rbnode_base const *restrict left,
    struct scc_rbnode_base const *restrict right
) {
    void const *laddr = scc_rbnode_value_qual(tree, left, const);
    void const *raddr = scc_rbnode_value_qual(tree, right, const);
    return tree->rb_compare(laddr, raddr);
}

static inline bool scc_rbtree_inspect_left_violation(
    struct scc_rbtree_base const *tree,
    struct scc_rbnode_base const *node
) {
    return scc_rbtree_inspect_has_child(node, SCC_RBTREE_INSPECT_LEFT) &&
           scc_rbtree_inspect_compare(tree, node->rn_left, node) >= 0;
}

static inline bool scc_rbtree_inspect_right_violation(
    struct scc_rbtree_base const *tree,
    struct scc_rbnode_base const *node
) {
    return scc_rbtree_inspect_has_child(node, SCC_RBTREE_INSPECT_RIGHT) &&
           scc_rbtree_inspect_compare(tree, node, node->rn_right) >= 0;
}

scc_inspect_mask scc_rbtree_inspect_node(
    struct scc_rbtree_base const *restrict tree,
    struct scc_rbnode_base const *restrict node
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

    return 0ull;
}


scc_inspect_mask scc_rbtree_inspect_properties(void const *handle) {
    enum { NOT_TRAVERSED = -1 };
    struct scc_rbtree_base const *tree = scc_rbtree_impl_base_qual(handle, const);

    if(!tree->rb_size) {
        return 0;
    }

    if(!scc_rbtree_inspect_black(tree->rb_root)) {
        /* Root must be black */
        return SCC_RBTREE_ERR_ROOT;
    }

    struct nodectx {
        struct scc_rbnode_base const *ct_node;
        long long *ct_pval;
        long long ct_left;
        long long ct_right;
    };

    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);

    scc_stack_push(&stack, ((struct nodectx) {
        .ct_node = tree->rb_root,
        .ct_left = NOT_TRAVERSED,
        .ct_right = NOT_TRAVERSED
    }));

    struct nodectx *curr;
    scc_inspect_mask emask = 0ull;
    while(!scc_stack_empty(stack)) {
        curr = &scc_stack_top(stack);
        if(curr->ct_left == NOT_TRAVERSED) {
            /* Height contribution of curr */
            curr->ct_left = scc_rbtree_inspect_black(curr->ct_node);
            if(scc_rbtree_inspect_has_child(curr->ct_node, SCC_RBTREE_INSPECT_LEFT)) {
                /* For computing height of left subtree */
                scc_stack_push(&stack, ((struct nodectx) {
                    .ct_node = curr->ct_node->rn_left,
                    .ct_pval = &curr->ct_left,
                    .ct_left = NOT_TRAVERSED,
                    .ct_right = NOT_TRAVERSED
                }));
            }
        }
        else if(curr->ct_right == NOT_TRAVERSED) {
            /* Height contribution of curr */
            curr->ct_right = scc_rbtree_inspect_black(curr->ct_node);
            if(scc_rbtree_inspect_has_child(curr->ct_node, SCC_RBTREE_INSPECT_RIGHT)) {
                /* For computing height of right subtree */
                scc_stack_push(&stack, ((struct nodectx) {
                    .ct_node = curr->ct_node->rn_right,
                    .ct_pval = &curr->ct_right,
                    .ct_left = NOT_TRAVERSED,
                    .ct_right = NOT_TRAVERSED
                }));
            }
        }
        else {
            if(curr->ct_left != curr->ct_right) {
                /* Black height violation in subtrees */
                emask = SCC_RBTREE_ERR_BLACK;
                goto epilogue;
            }
            if(curr->ct_pval) {
                /* Add black height of subtree to parent's counter */
                *curr->ct_pval += curr->ct_left;
            }
            scc_stack_pop(stack);
        }
    }

epilogue:
    scc_stack_free(stack);
    return emask;
}

void scc_rbtree_inspect_dump_flags(scc_inspect_mask flags) {
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
