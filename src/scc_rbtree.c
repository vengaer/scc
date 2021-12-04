#include <scc/scc_mem.h>
#include <scc/scc_rbtree.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>

#define rn_left     rn_bare.un_link.node.left
#define rn_right    rn_bare.un_link.node.right
#define rn_tree     rn_bare.un_link.tree
#define rn_color    rn_bare.color
#define rn_flags    rn_bare.flags

#define rb_root     rb_sentinel.un_link.root

#define scc_rbtree_sentinel_qual(tree, qual)        \
    (struct scc_rbnode qual *)&(tree)->rb_sentinel

#define scc_rbtree_sentinel(tree)                   \
    scc_rbtree_sentinel_qual(tree,)

#define scc_rbnode_link_qual(node, idx, qual)       \
    (*(struct scc_rbnode * qual*)                   \
        ((unsigned char qual*)&(node)->rn_left  +   \
            scc_rbnode_link_offset(node) * idx))

#define scc_rbnode_link(node, idx)                  \
    scc_rbnode_link_qual(node, idx,)

#define scc_rbnode_baseaddr_qual(node, qual)        \
    scc_container_qual(node - scc_rbnode_npad(node), struct scc_rbnode, rn_buffer, qual)

#define scc_rbnode_baseaddr(node)                   \
    scc_rbnode_baseaddr_qual(node,)

#define scc_rbtree_from_handle_qual(handle, qual)   \
    scc_rbnode_baseaddr_qual(handle, qual)->rn_tree

#define scc_rbtree_from_handle(handle)              \
    scc_rbtree_from_handle_qual(handle,)

#define scc_rbnode_value_qual(tree, node, qual)     \
    ((unsigned char qual *)(node) + (tree)->rb_baseoff)

#define scc_rbnode_value(tree, node)                \
    scc_rbnode_value_qual(tree, node,)

enum {
    SCC_RBLTHRD = 0x01,
    SCC_RBRTHRD = 0x02,
    SCC_RBLEAF  = SCC_RBLTHRD | SCC_RBRTHRD
};

enum scc_rbdir {
    scc_rbdir_left,
    scc_rbdir_right
};

bool scc_rbtree_empty(void const *handle);

static void scc_rbnode_set_npad(
    struct scc_rbnode *restrict node,
    struct scc_rbtree const *restrict tree
);
static unsigned scc_rbnode_npad(void const *rbnode);
static void *scc_rbtree_new_handle(struct scc_rbtree *tree);
static unsigned scc_rbnode_link_offset(struct scc_rbnode const *node);

static void scc_rbnode_mkred(struct scc_rbnode *node);
static void scc_rbnode_mkblack(struct scc_rbnode *node);
static void scc_rbnode_mkleaf(struct scc_rbnode *node);
static void scc_rbnode_set(struct scc_rbnode *node, enum scc_rbdir dir);
static void scc_rbnode_unset(struct scc_rbnode *node, enum scc_rbdir dir);
static bool scc_rbnode_thread(struct scc_rbnode const *node, enum scc_rbdir dir);
static void scc_rbnode_thread_from(
    struct scc_rbnode *restrict dst,
    struct scc_rbnode const *restrict src,
    enum scc_rbdir dir
);
static bool scc_rbnode_red(struct scc_rbnode const *node);
static bool scc_rbnode_red_safe(struct scc_rbnode const *node, enum scc_rbdir dir);
static bool scc_rbnode_children_red_safe(struct scc_rbnode const *node);
static bool scc_rbnode_has_red_child(struct scc_rbnode const *node);
static bool scc_rbnode_has_thread_link(struct scc_rbnode const *node);
static int scc_rbtree_compare(
    struct scc_rbtree const *restrict tree,
    struct scc_rbnode const *restrict left,
    struct scc_rbnode const *restrict right
);
static void const *scc_rbnode_leftmost(struct scc_rbnode const *node);
static void const *scc_rbnode_rightmost(struct scc_rbnode const *node);
static struct scc_rbnode *scc_rbtree_rotate_single(struct scc_rbnode *root, enum scc_rbdir dir);
static struct scc_rbnode *scc_rbtree_rotate_double(struct scc_rbnode *root, enum scc_rbdir dir);
static void scc_rbtree_balance_insertion(
    struct scc_rbnode *n,
    struct scc_rbnode *p,
    struct scc_rbnode *gp,
    struct scc_rbnode *ggp
);
static struct scc_rbnode *scc_rbtree_balance_removal(
    struct scc_rbnode *n,
    struct scc_rbnode *p,
    struct scc_rbnode *gp,
    enum scc_rbdir dir
);

static bool scc_rbtree_replace_handle(void **handle, struct scc_rbtree *tree);
static bool scc_rbtree_insert_empty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *n);
static bool scc_rbtree_insert_nonempty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *node);
static struct scc_rbnode *scc_rbtree_find_parent(struct scc_rbtree *tree, struct scc_rbnode const *needle);
static bool scc_rbnode_is_child_of(
    struct scc_rbnode const *restrict child,
    struct scc_rbnode const *restrict parent
);
static void scc_rbnode_swap(
    struct scc_rbtree *tree,
    struct scc_rbnode *found,
    struct scc_rbnode *n,
    struct scc_rbnode *p,
    enum scc_rbdir dir
);

static inline void scc_rbnode_set_npad(
    struct scc_rbnode *restrict node,
    struct scc_rbtree const *restrict tree
) {

    unsigned const npad = tree->rb_baseoff - sizeof(struct scc_rbnode_bare);
    assert(npad <= UCHAR_MAX);
    ((unsigned char *)node)[tree->rb_baseoff - 1] = npad;
}

static inline unsigned scc_rbnode_npad(void const *node) {
    return ((unsigned char const *)node)[-1];
}

static void *scc_rbtree_new_handle(struct scc_rbtree *tree) {
    struct scc_rbnode *node = scc_arena_alloc(&tree->rb_arena);
    if(!node) {
        return 0;
    }
    node->rn_tree = tree;
    scc_rbnode_set_npad(node, tree);
    return (unsigned char *)node + tree->rb_baseoff;
}

static inline unsigned scc_rbnode_link_offset(struct scc_rbnode const *node) {
    return (unsigned char const *)&node->rn_right - (unsigned char const *)&node->rn_left;
}

static inline void scc_rbnode_mkred(struct scc_rbnode *node) {
    node->rn_color = scc_rbcolor_red;
}

static inline void scc_rbnode_mkblack(struct scc_rbnode *node) {
    node->rn_color = scc_rbcolor_black;
}

static void scc_rbnode_mkleaf(struct scc_rbnode *node) {
    node->rn_flags = SCC_RBLEAF;
}

static inline void scc_rbnode_set(struct scc_rbnode *node, enum scc_rbdir dir) {
    node->rn_flags |= (1 << dir);
}

static inline void scc_rbnode_unset(struct scc_rbnode *node, enum scc_rbdir dir) {
    node->rn_flags &= ~(1 << dir);
}

static inline bool scc_rbnode_thread(struct scc_rbnode const *node, enum scc_rbdir dir) {
    return node->rn_flags & (1 << dir);
}

static void scc_rbnode_thread_from(
    struct scc_rbnode *restrict dst,
    struct scc_rbnode const *restrict src,
    enum scc_rbdir dir
) {
    dst->rn_flags = (dst->rn_flags & ~(1 << dir)) | (src->rn_flags & (1 << dir));
}

static inline bool scc_rbnode_red(struct scc_rbnode const *node) {
    return node->rn_color == scc_rbcolor_red;
}

static inline bool scc_rbnode_red_safe(struct scc_rbnode const *node, enum scc_rbdir dir) {
    return !scc_rbnode_thread(node, dir) &&
            scc_rbnode_red(scc_rbnode_link_qual(node, dir, const));
}

static inline bool scc_rbnode_children_red_safe(struct scc_rbnode const *node) {
    return scc_rbnode_red_safe(node, scc_rbdir_left) &&
           scc_rbnode_red_safe(node, scc_rbdir_right);
}

static inline bool scc_rbnode_has_red_child(struct scc_rbnode const *node) {
    return scc_rbnode_red_safe(node, scc_rbdir_left) ||
           scc_rbnode_red_safe(node, scc_rbdir_right);
}

static inline bool scc_rbnode_has_thread_link(struct scc_rbnode const *node) {
    return scc_rbnode_thread(node, scc_rbdir_left) ||
           scc_rbnode_thread(node, scc_rbdir_right);
}

static inline int scc_rbtree_compare(
    struct scc_rbtree const *restrict tree,
    struct scc_rbnode const *restrict left,
    struct scc_rbnode const *restrict right
) {
    void const *laddr = scc_rbnode_value_qual(tree, left, const);
    void const *raddr = scc_rbnode_value_qual(tree, right, const);
    return tree->rb_compare(laddr, raddr);
}

static void const *scc_rbnode_leftmost(struct scc_rbnode const *node) {
    while(!scc_rbnode_thread(node, scc_rbdir_left)) {
        node = node->rn_left;
    }
    return node;
}

static void const *scc_rbnode_rightmost(struct scc_rbnode const *node) {
    while(!scc_rbnode_thread(node, scc_rbdir_right)) {
        node = node->rn_right;
    }
    return node;
}

static struct scc_rbnode *scc_rbtree_rotate_single(struct scc_rbnode *root, enum scc_rbdir dir) {
    struct scc_rbnode *n = scc_rbnode_link(root, !dir);

    if(scc_rbnode_thread(n, dir)) {
        /* Links already correct, modify thread flags */
        scc_rbnode_set(root, !dir);
        scc_rbnode_unset(n, dir);
    }
    else {
        scc_rbnode_link(root, !dir) = scc_rbnode_link(n, dir);
    }
    scc_rbnode_link(n, dir) = root;

    scc_rbnode_mkred(root);
    scc_rbnode_mkblack(n);

    return n;
}

static struct scc_rbnode *scc_rbtree_rotate_double(struct scc_rbnode *root, enum scc_rbdir dir) {
    scc_rbnode_link(root, !dir) = scc_rbtree_rotate_single(scc_rbnode_link(root, !dir), !dir);
    return scc_rbtree_rotate_single(root, dir);
}

static void scc_rbtree_balance_insertion(
    struct scc_rbnode *n,
    struct scc_rbnode *p,
    struct scc_rbnode *gp,
    struct scc_rbnode *ggp
) {
    scc_rbnode_mkred(n);
    if(!scc_rbnode_has_thread_link(n)) {
        scc_rbnode_mkblack(n->rn_left);
        scc_rbnode_mkblack(n->rn_right);
    }

    if(scc_rbnode_red(p)) {
        scc_rbnode_mkred(gp);

        enum scc_rbdir pdir = p->rn_right == n;
        enum scc_rbdir gpdir = gp->rn_right == p;
        enum scc_rbdir ggpdir = ggp->rn_right == gp;

        if(pdir != gpdir) {
            /* No straight line, make leaf root */
            scc_rbnode_link(ggp, ggpdir) = scc_rbtree_rotate_double(gp, !gpdir);
            scc_rbnode_mkblack(n);
        }
        else {
            /* Straight line, make p root */
            scc_rbnode_link(ggp, ggpdir) = scc_rbtree_rotate_single(gp, !gpdir);
            scc_rbnode_mkblack(p);
        }
    }
}

static struct scc_rbnode *scc_rbtree_balance_removal(
    struct scc_rbnode *n,
    struct scc_rbnode *p,
    struct scc_rbnode *gp,
    enum scc_rbdir dir
) {
    enum scc_rbdir pdir = p->rn_right == n;
    enum scc_rbdir gpdir = gp->rn_right == p;

    if(scc_rbnode_red_safe(n, !dir)) {
        scc_rbnode_link(p, dir) = scc_rbtree_rotate_single(n, dir);
        return scc_rbnode_link(p, pdir);
    }

    if(!scc_rbnode_thread(p, !pdir)) {
        struct scc_rbnode *sibling = scc_rbnode_link(p, !pdir);
        if(scc_rbnode_has_red_child(sibling)) {
            if(scc_rbnode_red_safe(sibling, pdir)) {
                scc_rbnode_link(gp, gpdir) = scc_rbtree_rotate_double(p, dir);
            }
            else {
                scc_rbnode_link(gp, gpdir) = scc_rbtree_rotate_single(p, pdir);
            }

            scc_rbnode_mkred(n);
            scc_rbnode_mkred(scc_rbnode_link(gp, gpdir));
            scc_rbnode_mkblack(scc_rbnode_link(gp, gpdir)->rn_left);
            scc_rbnode_mkblack(scc_rbnode_link(gp, gpdir)->rn_right);
        }
        else {
            scc_rbnode_mkred(n);
            scc_rbnode_mkred(sibling);
            scc_rbnode_mkblack(p);
        }
    }
    return p;
}

static bool scc_rbtree_replace_handle(void **handle, struct scc_rbtree *tree) {
    void *new_handle = scc_rbtree_new_handle(tree);
    if(!new_handle) {
        return false;
    }
    *handle = new_handle;
    return true;
}

static bool scc_rbtree_insert_empty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *node) {
    if(!scc_rbtree_replace_handle(handle, tree)) {
        return false;
    }
    node->rn_left = scc_rbtree_sentinel(tree);
    node->rn_right = scc_rbtree_sentinel(tree);
    scc_rbnode_mkblack(node);
    scc_rbnode_mkleaf(node);

    tree->rb_root = node;
    scc_rbnode_unset(scc_rbtree_sentinel(tree), scc_rbdir_left);
    tree->rb_size = 1u;
    return true;
}

static bool scc_rbtree_insert_nonempty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *node) {
    struct scc_rbnode *gp =  &(struct scc_rbnode) { .rn_left = scc_rbtree_sentinel(tree) };
    struct scc_rbnode *ggp = &(struct scc_rbnode) { .rn_left = gp };
    struct scc_rbnode *p = scc_rbtree_sentinel(tree);
    struct scc_rbnode *n = tree->rb_root;

    enum scc_rbdir dir;
    int rel;

    while(1) {
        if(scc_rbnode_children_red_safe(n)) {
            /* Push red coloring up */
            scc_rbtree_balance_insertion(n, p, gp, ggp);
        }
        rel = scc_rbtree_compare(tree, n, node);
        if(!rel) {
            /* Already in tree */
            scc_rbnode_mkblack(tree->rb_root);
            return false;
        }
        dir = rel > 0;
        if(scc_rbnode_thread(n, dir)) {
            break;
        }

        ggp = gp;
        gp = p;
        p = n;
        n = scc_rbnode_link(n, dir);
    }

    if(!scc_rbtree_replace_handle(handle, tree)) {
        return false;
    }

    /* Prepare node for insertion */
    scc_rbnode_mkleaf(node);
    scc_rbnode_link(node, dir) = scc_rbnode_link(n, dir);
    scc_rbnode_link(node, !dir) = n;

    /* Set node as child of n */
    scc_rbnode_link(n, dir) = node;
    scc_rbnode_unset(n, dir);

    /* Uphold properties */
    scc_rbtree_balance_insertion(node, n, p, gp);
    scc_rbnode_mkblack(tree->rb_root);

    ++tree->rb_size;
    return true;
}

static struct scc_rbnode *scc_rbtree_find_parent(
    struct scc_rbtree *tree,
    struct scc_rbnode const *needle
) {
    int rel;
    struct scc_rbnode *p = scc_rbtree_sentinel(tree);
    struct scc_rbnode *n = tree->rb_root;
    enum scc_rbdir dir = scc_rbdir_left;
    while(!scc_rbnode_thread(p, dir) && n != needle) {
        rel = scc_rbtree_compare(tree, n, needle);
        dir = rel > 0;
        p = n;
        n = scc_rbnode_link(n, dir);
    }
    assert(p);
    return p;
}

static inline bool scc_rbnode_is_child_of(
    struct scc_rbnode const *restrict child,
    struct scc_rbnode const *restrict parent
) {
    return  parent->rn_left == child || parent->rn_right == child;
}

static void scc_rbnode_swap(
    struct scc_rbtree *tree,
    struct scc_rbnode *found,
    struct scc_rbnode *n,
    struct scc_rbnode *p,
    enum scc_rbdir dir
) {
    struct scc_rbnode *fparent = scc_rbnode_link(n, dir);
    /* If balancing has separated parent and child, look up new parent */
    if(!scc_rbnode_is_child_of(found, fparent)) {
        fparent = scc_rbtree_find_parent(tree, found);
    }

    enum scc_rbdir fdir = fparent->rn_right == found;
    enum scc_rbdir pdir = p->rn_right == n;

    /* Adopt n's flag in direction pdir */
    scc_rbnode_thread_from(p, n, pdir);

    /* Replace found with n */
    struct scc_rbnode *sentinel = scc_rbtree_sentinel(tree);
    n->rn_left = found->rn_left == n ? sentinel : found->rn_left;
    n->rn_right = found->rn_right == n ? sentinel : found->rn_right;
    n->rn_color = found->rn_color;
    n->rn_flags = found->rn_flags;

    scc_rbnode_link(fparent, fdir) = n;
    scc_rbnode_link(p, pdir) = scc_rbnode_link(n, dir);
}

void *scc_rbtree_impl_init(struct scc_rbtree *tree) {
    tree->rb_size = 0u;
    tree->rb_root = scc_rbtree_sentinel(tree);
    tree->rb_sentinel.flags = SCC_RBLEAF;

    return scc_rbtree_new_handle(tree);
}

void scc_rbtree_free(void *handle) {
    scc_arena_release(&scc_rbtree_from_handle(handle)->rb_arena);
}

size_t scc_rbtree_size(void const *handle) {
    return scc_rbtree_from_handle_qual(handle, const)->rb_size;
}

bool scc_rbtree_impl_insert(void *handle) {
    struct scc_rbtree *tree = scc_rbtree_from_handle(*(void **)handle);
    struct scc_rbnode *node = scc_rbnode_baseaddr(*(void **)handle);

    if(scc_rbtree_empty(*(void **)handle)) {
        return scc_rbtree_insert_empty(handle, tree, node);
    }

    return scc_rbtree_insert_nonempty(handle, tree, node);
}

void const *scc_rbtree_impl_find(void const *handle) {
    struct scc_rbtree const *tree = scc_rbtree_from_handle_qual(handle, const);
    struct scc_rbnode const *p = scc_rbtree_sentinel_qual(tree, const);
    struct scc_rbnode const *n = scc_rbtree_sentinel_qual(tree, const);
    struct scc_rbnode const *needle = scc_rbnode_baseaddr_qual(handle, const);
    enum scc_rbdir dir = scc_rbdir_left;
    int rel;

    while(!scc_rbnode_thread(p, dir)) {
        rel = scc_rbtree_compare(tree, n, needle);
        if(!rel) {
            return scc_rbnode_value_qual(tree, n, const);
        }

        dir = rel > 0;
        p = n;
        n = scc_rbnode_link_qual(n, dir, const);
    }

    return 0;
}

bool scc_rbtree_impl_remove(void *handle) {
    struct scc_rbtree *tree = scc_rbtree_from_handle(handle);
    struct scc_rbnode const *needle = scc_rbnode_baseaddr(handle);
    struct scc_rbnode *gp = &(struct scc_rbnode) {
        .rn_left = scc_rbtree_sentinel(tree)
    };
    struct scc_rbnode *p = scc_rbtree_sentinel(tree);
    struct scc_rbnode *n = tree->rb_root;

    struct scc_rbnode *fparent = 0;
    struct scc_rbnode *found = 0;

    enum scc_rbdir dir = scc_rbdir_left;
    int rel;

    while(!scc_rbnode_thread(p, dir)) {
        rel = scc_rbtree_compare(tree, n, needle);
        if(!rel) {
            fparent = p;
            found = n;
        }

        dir = rel > 0;

        if(!scc_rbnode_red(n) && !scc_rbnode_red_safe(n, dir)) {
            p = scc_rbtree_balance_removal(n, p, gp, dir);
        }

        gp = p;
        p = n;
        n = scc_rbnode_link(n, dir);
    }

    if(found) {
        /* Use thread for passing fparent */
        scc_rbnode_link(p, dir) = fparent;
        scc_rbnode_swap(tree, found, p, gp, dir);
        scc_arena_free(&tree->rb_arena, found);
        --tree->rb_size;
    }
    if(!scc_rbnode_thread(scc_rbtree_sentinel(tree), scc_rbdir_left)) {
        scc_rbnode_mkblack(tree->rb_root);
    }

    return found;
}

void const *scc_rbtree_impl_leftmost(void const *handle) {
    struct scc_rbtree const *tree = scc_rbtree_from_handle_qual(handle, const);
    struct scc_rbnode const *leftmost = scc_rbnode_leftmost(tree->rb_root);
    return scc_rbnode_value_qual(tree, leftmost, const);
}

void const *scc_rbtree_impl_rightmost(void const *handle) {
    struct scc_rbtree const *tree = scc_rbtree_from_handle_qual(handle, const);
    struct scc_rbnode const *rightmost = scc_rbnode_rightmost(tree->rb_root);
    return scc_rbnode_value_qual(tree, rightmost, const);
}

void const *scc_rbtree_impl_successor(void const *iter) {
    struct scc_rbnode const *node = scc_rbnode_baseaddr_qual(iter, const);
    size_t const offset = (unsigned char const *)iter - (unsigned char const *)node;
    if(scc_rbnode_thread(node, scc_rbdir_right)) {
        node = node->rn_right;
    }
    else {
        node = scc_rbnode_leftmost(node->rn_right);
    }
    return (unsigned char const *)node + offset;
}

void const *scc_rbtree_impl_predecessor(void const *iter) {
    struct scc_rbnode const *node = scc_rbnode_baseaddr_qual(iter, const);
    size_t const offset = (unsigned char const *)iter - (unsigned char const *)node;
    if(scc_rbnode_thread(node, scc_rbdir_left)) {
        node = node->rn_left;
    }
    else {
        node = scc_rbnode_rightmost(node->rn_left);
    }
    return (unsigned char const *)node + offset;
}

void const *scc_rbtree_impl_sentinel(void const *handle) {
    struct scc_rbtree const *tree = scc_rbtree_from_handle_qual(handle, const);
    struct scc_rbnode const *sentinel = scc_rbtree_sentinel_qual(tree, const);
    return scc_rbnode_value_qual(tree, sentinel, const);
}
