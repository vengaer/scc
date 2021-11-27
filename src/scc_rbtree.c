#include <scc/scc_mem.h>
#include <scc/scc_rbtree.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>

#define rn_left     rn_bare.un_link.node.left
#define rn_right    rn_bare.un_link.node.right
#define rn_parent   rn_bare.un_link.temp.parent
#define rn_color    rn_bare.color
#define rn_flags    rn_bare.flags

#define scc_rbtree_root(tree)                       \
    ((tree)->rb_sentinel.un_link.node.left)

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
    scc_rbnode_baseaddr_qual(handle, qual)->rn_parent

#define scc_rbtree_from_handle(handle)              \
    scc_rbtree_from_handle_qual(handle,)

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

static void scc_rbnode_mkred(void *node);
static void scc_rbnode_mkblack(void *node);
static void scc_rbnode_mkleaf(void *node);
static void scc_rbnode_set(void *node, enum scc_rbdir dir);
static void scc_rbnode_unset(void *node, enum scc_rbdir dir);
static bool scc_rbnode_thread(struct scc_rbnode const *node, enum scc_rbdir dir);
static bool scc_rbnode_red(struct scc_rbnode const *node);
static bool scc_rbnode_red_safe(struct scc_rbnode const *node, enum scc_rbdir dir);
static bool scc_rbnode_children_red_safe(struct scc_rbnode const *node);
static bool scc_rbnode_has_thread_link(struct scc_rbnode const *node);
static int scc_rbtree_compare(
    struct scc_rbtree const *restrict tree,
    struct scc_rbnode const *restrict left,
    struct scc_rbnode const *restrict right
);

static bool scc_rbtree_replace_handle(void **handle, struct scc_rbtree *tree);
static bool scc_rbtree_insert_empty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *n);

static struct scc_rbnode *scc_rbtree_rotate_single(struct scc_rbnode *root, enum scc_rbdir dir);
static struct scc_rbnode *scc_rbtree_rotate_double(struct scc_rbnode *root, enum scc_rbdir dir);
static bool scc_rbtree_insert_nonempty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *node);

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
    node->rn_parent = tree;
    scc_rbnode_set_npad(node, tree);
    return (unsigned char *)node + tree->rb_baseoff;
}

static inline unsigned scc_rbnode_link_offset(struct scc_rbnode const *node) {
    return (unsigned char const *)&node->rn_right - (unsigned char const *)&node->rn_left;
}

static inline void scc_rbnode_mkred(void *node) {
    ((struct scc_rbnode *)node)->rn_color = scc_rbcolor_red;
}

static inline void scc_rbnode_mkblack(void *node) {
    ((struct scc_rbnode *)node)->rn_color = scc_rbcolor_black;
}

static void scc_rbnode_mkleaf(void *node) {
    ((struct scc_rbnode *)node)->rn_flags = SCC_RBLEAF;
}

static inline void scc_rbnode_set(void *node, enum scc_rbdir dir) {
    ((struct scc_rbnode *)node)->rn_flags |= (1 << dir);
}

static inline void scc_rbnode_unset(void *node, enum scc_rbdir dir) {
    ((struct scc_rbnode *)node)->rn_flags &= ~(1 << dir);
}

static inline bool scc_rbnode_thread(struct scc_rbnode const *node, enum scc_rbdir dir) {
    return node->rn_flags & (1 << dir);
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

static inline bool scc_rbnode_has_thread_link(struct scc_rbnode const *node) {
    return scc_rbnode_thread(node, scc_rbdir_left) ||
           scc_rbnode_thread(node, scc_rbdir_right);
}

static inline int scc_rbtree_compare(
    struct scc_rbtree const *restrict tree,
    struct scc_rbnode const *restrict left,
    struct scc_rbnode const *restrict right
) {
    void const *laddr = (unsigned char const *)left + tree->rb_baseoff;
    void const *raddr = (unsigned char const *)right + tree->rb_baseoff;
    return tree->rb_compare(laddr, raddr);
}

static bool scc_rbtree_replace_handle(void **handle, struct scc_rbtree *tree) {
    void *new_handle = scc_rbtree_new_handle(tree);
    if(!new_handle) {
        return false;
    }
    *(void **)handle = new_handle;
    return true;
}

static bool scc_rbtree_insert_empty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *node) {
    if(!scc_rbtree_replace_handle(handle, tree)) {
        return false;
    }
    node->rn_left = &tree->rb_sentinel;
    node->rn_right = &tree->rb_sentinel;
    scc_rbnode_mkblack(node);
    scc_rbnode_mkleaf(node);

    scc_rbtree_root(tree) = (struct scc_rbnode_bare *)node;
    scc_rbnode_unset(&tree->rb_sentinel, scc_rbdir_left);
    tree->rb_size = 1u;
    return true;
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

        enum scc_rbdir pdir = p->rn_right == (void *)n;
        enum scc_rbdir gpdir = gp->rn_right == (void *)p;
        enum scc_rbdir ggpdir = ggp->rn_right == (void *)gp;

        if(pdir != gpdir) {
            /* No straight line, double rotate */
            scc_rbnode_link(ggp, ggpdir) = scc_rbtree_rotate_double(gp, !gpdir);
            scc_rbnode_mkblack(n);
        }
        else {
            scc_rbnode_link(ggp, ggpdir) = scc_rbtree_rotate_single(gp, !gpdir);
            scc_rbnode_mkblack(p);
        }
    }
}

static bool scc_rbtree_insert_nonempty(void **handle, struct scc_rbtree *tree, struct scc_rbnode *node) {
    struct scc_rbnode *gp =  &(struct scc_rbnode) { .rn_left = &tree->rb_sentinel };
    struct scc_rbnode *ggp = &(struct scc_rbnode) { .rn_left = (struct scc_rbnode_bare *)gp };
    struct scc_rbnode *p = (struct scc_rbnode *)&tree->rb_sentinel;
    struct scc_rbnode *n = (struct scc_rbnode *)scc_rbtree_root(tree);

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
            scc_rbnode_mkblack(scc_rbtree_root(tree));
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
    scc_rbnode_mkblack(scc_rbtree_root(tree));

    ++tree->rb_size;
    return true;
}

void *scc_rbtree_impl_init(struct scc_rbtree *tree) {
    tree->rb_size = 0u;
    tree->rb_sentinel.un_link.node.left = &tree->rb_sentinel;
    tree->rb_sentinel.un_link.node.right = &tree->rb_sentinel;
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
