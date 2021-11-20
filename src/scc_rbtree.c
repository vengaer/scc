#include <scc/scc_rbtree.h>

#include <stdbool.h>

#define scc_rbtree_link_qual(node, idx, qual)                               \
    (*(struct scc_rbnode *qual *)((unsigned char qual *)&(node)->rn_left +  \
        ((unsigned char qual *)&(node)->rn_right -                          \
            (unsigned char qual *)&(node)->rn_left) * (idx)))               \

#define scc_rbtree_link(node, idx)                                          \
    scc_rbtree_link_qual(node, idx,)

struct scc_rbtree *scc_rbtree_impl_init(struct scc_rbtree *tree, scc_rbcompare compare);
_Bool scc_rbtree_empty(struct scc_rbtree const *tree);

static void scc_rbnode_mk_red(struct scc_rbnode *node);
static void scc_rbnode_mk_black(struct scc_rbnode *node);
static void scc_rbnode_mk_leaf(struct scc_rbnode *node);
static bool scc_rbnode_red(struct scc_rbnode const *node);
static bool scc_rbnode_thread(struct scc_rbnode const *node, enum scc_rbdir dir);
static void scc_rbnode_set(struct scc_rbnode *node, enum scc_rbdir dir);
static void scc_rbnode_unset(struct scc_rbnode *node, enum scc_rbdir dir);
static bool scc_rbnode_red_safe(struct scc_rbnode const *node, enum scc_rbdir dir);
static bool scc_rbnode_children_red_safe(struct scc_rbnode const *node);
static struct scc_rbnode *scc_rbtree_rotate_single(struct scc_rbnode *root, enum scc_rbdir dir);
static struct scc_rbnode *scc_rbtree_rotate_double(struct scc_rbnode *root, enum scc_rbdir dir);
static void scc_rbtree_balance_insertion(
    struct scc_rbnode *restrict n,
    struct scc_rbnode *restrict p,
    struct scc_rbnode *restrict gp,
    struct scc_rbnode *restrict ggp
);

static inline void scc_rbnode_mk_red(struct scc_rbnode *node) {
    node->rn_color = scc_rbcolor_red;
}

static inline void scc_rbnode_mk_black(struct scc_rbnode *node) {
    node->rn_color = scc_rbcolor_black;
}

static inline void scc_rbnode_mk_leaf(struct scc_rbnode *node) {
    node->rn_flags = SCC_RBTHRD_LEAF;
}

static inline bool scc_rbnode_red(struct scc_rbnode const *node) {
    return node->rn_color == scc_rbcolor_red;
}

static inline bool scc_rbnode_thread(struct scc_rbnode const *node, enum scc_rbdir dir) {
    return node->rn_flags & (1 << dir);
}

static inline void scc_rbnode_set(struct scc_rbnode *node, enum scc_rbdir dir) {
    node->rn_flags |= (1 << dir);
}

static inline void scc_rbnode_unset(struct scc_rbnode *node, enum scc_rbdir dir) {
    node->rn_flags &= ~(1 << dir);
}

static inline bool scc_rbnode_red_safe(struct scc_rbnode const *node, enum scc_rbdir dir) {
    return !scc_rbnode_thread(node, dir) && scc_rbnode_red(scc_rbtree_link_qual(node, dir, const));
}

static inline bool scc_rbnode_children_red_safe(struct scc_rbnode const *node) {
    return scc_rbnode_red_safe(node, scc_rbdir_left) &&
           scc_rbnode_red_safe(node, scc_rbdir_right);
}

static struct scc_rbnode *scc_rbtree_rotate_single(struct scc_rbnode *root, enum scc_rbdir dir) {
    struct scc_rbnode *n = scc_rbtree_link(root, !dir);

    if(scc_rbnode_thread(n, dir)) {
        /* Links already referring to correct nodes */
        scc_rbnode_set(root, !dir);
        scc_rbnode_unset(root, dir);
    }
    else {
        scc_rbtree_link(root, !dir) = scc_rbtree_link(n, dir);
    }
    scc_rbtree_link(n, dir) = root;

    scc_rbnode_mk_red(root);
    scc_rbnode_mk_black(n);

    return n;
}

static inline struct scc_rbnode *scc_rbtree_rotate_double(struct scc_rbnode *root, enum scc_rbdir dir) {
    scc_rbtree_link(root, !dir) = scc_rbtree_rotate_single(scc_rbtree_link(root, !dir), !dir);
    return scc_rbtree_rotate_single(root, dir);
}

static void scc_rbtree_balance_insertion(
    struct scc_rbnode *restrict n,
    struct scc_rbnode *restrict p,
    struct scc_rbnode *restrict gp,
    struct scc_rbnode *restrict ggp
) {
    enum scc_rbdir pdir;
    enum scc_rbdir gpdir;
    enum scc_rbdir ggpdir;

    scc_rbnode_mk_red(n);

    if(!scc_rbnode_thread(n, scc_rbdir_left) && !scc_rbnode_thread(n, scc_rbdir_right)) {
        scc_rbnode_mk_black(n->rn_left);
        scc_rbnode_mk_black(n->rn_right);
    }

    if(scc_rbnode_red(p)) {
        scc_rbnode_mk_red(gp);

        pdir = p->rn_right == n;
        gpdir = gp->rn_right == p;
        ggpdir = ggp->rn_right == gp;

        if(pdir != gpdir) {
            /* Not in a straight line, double rotation */
            scc_rbtree_link(ggp, ggpdir) = scc_rbtree_rotate_double(gp, !gpdir);
            scc_rbnode_mk_black(n);
        }
        else {
            /* Straight line, single rotation */
            scc_rbtree_link(ggp, ggpdir) = scc_rbtree_rotate_single(gp, !gpdir);
            scc_rbnode_mk_black(p);
        }
    }
}

bool scc_rbtree_insert(struct scc_rbtree *restrict tree, struct scc_rbnode *restrict node) {
    struct scc_rbnode *ggp;
    struct scc_rbnode *gp;
    struct scc_rbnode *p;
    struct scc_rbnode *n;

    enum scc_rbdir dir;
    int relation;

    if(scc_rbtree_empty(tree)) {
        node->rn_left = &tree->rt_sentinel;
        node->rn_right = &tree->rt_sentinel;
        node->rn_color = scc_rbcolor_black;
        node->rn_flags = SCC_RBTHRD_LEAF;
        scc_rbtree_impl_root(tree) = node;
        scc_rbnode_unset(&tree->rt_sentinel, scc_rbdir_left);
        tree->rt_sentinel.rn_right = &tree->rt_sentinel;
        tree->rt_size = 1u;
        return true;
    }

    gp = &(struct scc_rbnode){ .rn_left = &tree->rt_sentinel };
    ggp = &(struct scc_rbnode){ .rn_left = gp };
    p = &tree->rt_sentinel;
    n = scc_rbtree_impl_root(tree);

    while(true) {
        if(scc_rbnode_children_red_safe(n)) {
            /* Percolate red coloring upwards */
            scc_rbtree_balance_insertion(n, p, gp, ggp);
        }
        relation = tree->rt_compare(n, node);

        if(!relation) {
            /* Already in tree */
            scc_rbnode_mk_black(scc_rbtree_impl_root(tree));
            return false;
        }

        dir = relation > 0;
        if(scc_rbnode_thread(n, dir)) {
            break;
        }

        ggp = gp;
        gp = p;
        p = n;
        n = scc_rbtree_link(n, dir);
    }

    /* Prep node for insertion */
    scc_rbnode_mk_leaf(node);
    scc_rbtree_link(node, dir) = scc_rbtree_link(n, dir);
    scc_rbtree_link(node, !dir) = n;

    /* Set child of n */
    scc_rbtree_link(n, dir) = node;
    scc_rbnode_unset(n, dir);

    /* Ensure properties are upheld */
    scc_rbtree_balance_insertion(node, n, p, gp);
    scc_rbnode_mk_black(scc_rbtree_impl_root(tree));

    ++tree->rt_size;
    return true;
}
