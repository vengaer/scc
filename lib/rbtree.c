#include <scc/deque.h>
#include <scc/mem.h>
#include <scc/rbtree.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define rb_root rb_sentinel.rs_left

size_t scc_rbtree_impl_npad(void const *rbtree);
size_t scc_rbtree_size(void const *rbtree);
_Bool scc_rbtree_empty(void const *rbtree);
void const *scc_rbtree_impl_iterstop(void const *rbtree);
size_t scc_rbnode_link_offset(struct scc_rbnode_base const *node);
_Bool scc_rbnode_thread(struct scc_rbnode_base const *node, enum scc_rbdir dir);
size_t scc_rbnode_bkoff(void const *iter);
_Bool scc_rbtree_impl_insert(void *rbtreeaddr, size_t elemsize);

enum {
    SCC_RBLTHRD = 0x01,
    SCC_RBRTHRD = 0x02,
    SCC_RBLEAF = SCC_RBLTHRD | SCC_RBRTHRD
};

static inline void scc_rbtree_set_bkoff(unsigned char *rbtree, unsigned char bkoff) {
    rbtree[-1] = bkoff;
}

static inline void scc_rbnode_set_bkoff(
    struct scc_rbtree_base const *restrict base,
    struct scc_rbnode_base *restrict node
) {
    unsigned const bkoff = base->rb_dataoff - offsetof(struct scc_rbnode_base, rn_data);
    assert(bkoff <= UCHAR_MAX);
    ((unsigned char *)node)[base->rb_dataoff - 1u] = bkoff;
}

static inline void scc_rbnode_set(struct scc_rbnode_base *node, enum scc_rbdir dir) {
    node->rn_flags |= (1 << dir);
}

static inline void scc_rbnode_unset(struct scc_rbnode_base *node, enum scc_rbdir dir) {
    node->rn_flags &= ~(1 << dir);
}

static inline void scc_rbnode_thread_from(
    struct scc_rbnode_base *restrict dst,
    struct scc_rbnode_base const *restrict src,
    enum scc_rbdir dir
) {
    dst->rn_flags = (dst->rn_flags & ~(1 << dir)) | (src->rn_flags & (1 << dir));
}

static inline _Bool scc_rbnode_has_thread_link(struct scc_rbnode_base const *node) {
    return scc_rbnode_thread(node, scc_rbdir_left) ||
           scc_rbnode_thread(node, scc_rbdir_right);
}

static inline _Bool scc_rbnode_red(struct scc_rbnode_base const *node) {
    return node->rn_color == scc_rbcolor_red;
}

static inline _Bool scc_rbnode_red_safe(struct scc_rbnode_base const *node, enum scc_rbdir dir) {
    return !scc_rbnode_thread(node, dir) &&
            scc_rbnode_red(scc_rbnode_link_qual(node, dir, const));
}

static inline _Bool scc_rbnode_children_red_safe(struct scc_rbnode_base const *node) {
    return scc_rbnode_red_safe(node, scc_rbdir_left) &&
           scc_rbnode_red_safe(node, scc_rbdir_right);
}

static inline _Bool scc_rbnode_has_red_child(struct scc_rbnode_base const *node) {
    return scc_rbnode_red_safe(node, scc_rbdir_left) ||
           scc_rbnode_red_safe(node, scc_rbdir_right);
}

static inline void scc_rbnode_mkblack(struct scc_rbnode_base *node) {
    node->rn_color = scc_rbcolor_black;
}

static inline void scc_rbnode_mkred(struct scc_rbnode_base *node) {
    node->rn_color = scc_rbcolor_red;
}

static inline void scc_rbnode_mkleaf(struct scc_rbnode_base *node) {
    node->rn_flags = SCC_RBLEAF;
}

static inline int scc_rbtree_compare(
    struct scc_rbtree_base const *restrict base,
    struct scc_rbnode_base const *restrict node,
    void const *restrict value
) {
    return base->rb_compare(scc_rbnode_value_qual(base, node, const), value);
}

static struct scc_rbnode_base *scc_rbtree_rotate_single(struct scc_rbnode_base *root, enum scc_rbdir dir) {
    struct scc_rbnode_base *n = scc_rbnode_link(root, !dir);

    if (scc_rbnode_thread(n, dir)) {
        /* Links already coprrect, modify thread flags */
        scc_rbnode_set(root, !dir);
        scc_rbnode_unset(n, dir);
    }
    else {
        /* Must rotate */
        scc_rbnode_link(root, !dir) = scc_rbnode_link(n, dir);
        scc_rbnode_link(n, dir) = root;
    }

    scc_rbnode_mkred(root);
    scc_rbnode_mkblack(n);

    return n;
}

static inline struct scc_rbnode_base *scc_rbtree_rotate_double(struct scc_rbnode_base *root, enum scc_rbdir dir) {
    scc_rbnode_link(root, !dir) = scc_rbtree_rotate_single(scc_rbnode_link(root, !dir), !dir);
    return scc_rbtree_rotate_single(root, dir);
}

static void scc_rbtree_balance_insertion(
    struct scc_rbnode_base *n,
    struct scc_rbnode_base *p,
    struct scc_rbnode_base *gp,
    struct scc_rbnode_base *ggp
) {
    scc_rbnode_mkred(n);
    if (!scc_rbnode_has_thread_link(n)) {
        scc_rbnode_mkblack(n->rn_left);
        scc_rbnode_mkblack(n->rn_right);
    }

    if (scc_rbnode_red(p)) {
        scc_rbnode_mkred(gp);

        enum scc_rbdir pdir = p->rn_right == n;
        enum scc_rbdir gpdir = gp->rn_right == p;
        enum scc_rbdir ggpdir = ggp->rn_right == gp;

        if (pdir != gpdir) {
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

static struct scc_rbnode_base *scc_rbtree_balance_removal(
    struct scc_rbnode_base *n,
    struct scc_rbnode_base *p,
    struct scc_rbnode_base *gp,
    enum scc_rbdir dir
) {
    enum scc_rbdir pdir = p->rn_right == n;
    enum scc_rbdir gpdir = gp->rn_right == p;

    if (scc_rbnode_red_safe(n, !dir)) {
        scc_rbnode_link(p, pdir) = scc_rbtree_rotate_single(n, dir);
        return scc_rbnode_link(p, pdir);
    }

    if (!scc_rbnode_thread(p, !pdir)) {
        struct scc_rbnode_base *sibling = scc_rbnode_link(p, !pdir);
        if (scc_rbnode_has_red_child(sibling)) {
            if (scc_rbnode_red_safe(sibling, pdir)) {
                scc_rbnode_link(gp, gpdir) = scc_rbtree_rotate_double(p, pdir);
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

static inline struct scc_rbnode_base *scc_rbnode_new(
    struct scc_rbtree_base *restrict base,
    void const *restrict value,
    size_t elemsize
) {
    struct scc_rbnode_base *node = scc_arena_alloc(&base->rb_arena);
    if (!node) {
        return 0;
    }
    memcpy(scc_rbnode_value(base, node), value, elemsize);
    scc_rbnode_set_bkoff(base, node);
    return node;
}

static inline struct scc_rbnode_base const *scc_rbtree_leftmost(struct scc_rbnode_base const *root) {
    while (!scc_rbnode_thread(root, scc_rbdir_left)) {
        root = root->rn_left;
    }
    return root;
}

static inline struct scc_rbnode_base const *scc_rbtree_rightmost(struct scc_rbnode_base const *root) {
    while (!scc_rbnode_thread(root, scc_rbdir_right)) {
        root = root->rn_right;
    }
    return root;
}

static inline _Bool scc_rbtree_insert_empty(struct scc_rbtree_base *restrict base, void *restrict handle, size_t elemsize) {
    struct scc_rbnode_base *node = scc_rbnode_new(base, handle, elemsize);
    if (!node) {
        return false;
    }
    node->rn_left = (void *)&base->rb_sentinel;
    node->rn_right = (void *)&base->rb_sentinel;
    scc_rbnode_mkblack(node);
    scc_rbnode_mkleaf(node);
    base->rb_root = node;
    base->rb_size = 1u;
    scc_rbnode_unset((void *)&base->rb_sentinel, scc_rbdir_left);
    return true;
}

static void *scc_rbtree_insert_nonempty(struct scc_rbtree_base *restrict base, void *handle, size_t elemsize) {
    struct scc_rbnode_base *n = base->rb_root;
    struct scc_rbnode_base *p = (void *)&base->rb_sentinel;
    struct scc_rbnode_base *gp = &(struct scc_rbnode_base) { .rn_left = p };
    struct scc_rbnode_base *ggp = &(struct scc_rbnode_base) { .rn_left = gp };

    enum scc_rbdir dir;
    int rel;

    while (1) {
        if (scc_rbnode_children_red_safe(n)) {
            /* Push red coloring towards root */
            scc_rbtree_balance_insertion(n, p, gp, ggp);
        }
        rel = scc_rbtree_compare(base, n, handle);
        if (!rel) {
            /* Already in tree */
            scc_rbnode_mkblack(base->rb_root);
            return scc_rbnode_value(base, n);
        }
        dir = rel < 1;
        if (scc_rbnode_thread(n, dir)) {
            break;
        }

        ggp = gp;
        gp = p;
        p = n;
        n = scc_rbnode_link(n, dir);
    }

    /* Allocate */
    struct scc_rbnode_base *new = scc_rbnode_new(base, handle, elemsize);
    if (!new) {
        scc_rbnode_mkblack(base->rb_root);
        return 0;
    }

    /* Prepare node for insertion */
    scc_rbnode_mkleaf(new);
    scc_rbnode_link(new, dir) = scc_rbnode_link(n, dir);
    scc_rbnode_link(new, !dir) = n;

    /* Set node as child of n */
    scc_rbnode_link(n, dir) = new;
    scc_rbnode_unset(n, dir);

    /* Uphold properties */
    scc_rbtree_balance_insertion(new, n, p, gp);
    scc_rbnode_mkblack(base->rb_root);

    ++base->rb_size;
    return handle;
}

static inline struct scc_rbtree_base *scc_rbtree_clone_base(struct scc_rbtree_base const *obase, size_t elemsize, size_t basesz) {
    size_t bytesz = basesz + elemsize;
    scc_when_mutating(assert(bytesz > basesz));

    struct scc_rbtree_base *nbase = malloc(bytesz);
    if (!nbase) {
        return 0;
    }
    scc_memcpy(nbase, obase, basesz);
    nbase->rb_arena = scc_arena_clone(&obase->rb_arena);
    nbase->rb_dynalloc = 1;
    return nbase;
}

void *scc_rbtree_impl_new(struct scc_rbtree_base *base, size_t coff) {
    base->rb_size = 0u;
    base->rb_root = (void *)&base->rb_sentinel;

    /* Size field used to determine whether sentinel has a left child
     * or not. Unset left thread here to save a few ops */
    base->rb_sentinel.rs_flags = SCC_RBRTHRD;

    size_t fwoff = coff - offsetof(struct scc_rbtree_base, rb_fwoff) - sizeof(base->rb_fwoff);
    assert(fwoff <= UCHAR_MAX);
    base->rb_fwoff = (unsigned char)fwoff;
    unsigned char *rbtree = (unsigned char *)base + coff;
    scc_rbtree_set_bkoff(rbtree, fwoff);
    return rbtree;
}

void *scc_rbtree_impl_new_dyn(size_t treesz, struct scc_arena *arena, scc_rbcompare compare, size_t coff, size_t dataoff) {
    struct scc_rbtree_base *base = calloc(treesz, sizeof(unsigned char));
    if (!base) {
        return 0;
    }

    base->rb_arena = *arena;
    base->rb_compare = compare;
    base->rb_dataoff = dataoff;
    void *tree = scc_rbtree_impl_new(base, coff);
    base->rb_dynalloc = 1;
    return tree;
}

void scc_rbtree_clear(void *rbtree) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(rbtree);
    scc_arena_reset(&base->rb_arena);
    base->rb_size = 0u;
}

void scc_rbtree_free(void *rbtree) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(rbtree);
    scc_arena_release(&base->rb_arena);
    if (base->rb_dynalloc) {
        free(base);
    }
}

void *scc_rbtree_impl_generic_insert(void *rbtreeaddr, size_t elemsize) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(*(void **)rbtreeaddr);
    if (!base->rb_size) {
        if (scc_rbtree_insert_empty(base, *(void **)rbtreeaddr, elemsize)) {
            return *(void **)rbtreeaddr;
        }
        return 0;
    }

    return scc_rbtree_insert_nonempty(base, *(void **)rbtreeaddr, elemsize);
}

void const *scc_rbtree_impl_find(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    struct scc_rbnode_base const *p = (void const *)&base->rb_sentinel;
    struct scc_rbnode_base *n = base->rb_root;

    enum scc_rbdir dir = scc_rbdir_left;
    int rel;

    while (!scc_rbnode_thread(p, dir)) {
        rel = scc_rbtree_compare(base, n, rbtree);
        if (!rel) {
            return scc_rbnode_value_qual(base, n, const);
        }

        dir = rel < 1;
        p = n;
        n = scc_rbnode_link_qual(n, dir, const);
    }

    return 0;
}

_Bool scc_rbtree_impl_remove(void *rbtree, size_t elemsize) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(rbtree);

    struct scc_rbnode_base *n = base->rb_root;
    struct scc_rbnode_base *p = (void *)&base->rb_sentinel;
    struct scc_rbnode_base *gp = &(struct scc_rbnode_base) { .rn_left = p, };

    struct scc_rbnode_base *found = 0;

    enum scc_rbdir dir = scc_rbdir_left;
    int rel;

    while (!scc_rbnode_thread(p, dir)) {
        rel = scc_rbtree_compare(base, n, rbtree);
        if (!rel) {
            found = n;
        }

        dir = rel < 1;

        if (!scc_rbnode_red(n) && !scc_rbnode_red_safe(n, dir)) {
            p = scc_rbtree_balance_removal(n, p, gp, dir);
        }

        gp = p;
        p = n;
        n = scc_rbnode_link(n, dir);
    }

    if (found) {
        enum scc_rbdir gpdir = gp->rn_right == p;

        /* Replace value of found with value of p */
        scc_rbnode_thread_from(gp, p, gpdir);
        scc_rbnode_link(gp, gpdir) = scc_rbnode_link(p, gpdir);
        memcpy(scc_rbnode_value(base, found), scc_rbnode_value(base, p), elemsize);

        scc_arena_free(&base->rb_arena, p);
        --base->rb_size;
    }

    scc_rbnode_mkblack(base->rb_root);

    return found;
}

void const *scc_rbtree_impl_leftmost_value(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    struct scc_rbnode_base const *leftmost = scc_rbtree_leftmost(base->rb_root);
    return scc_rbnode_value_qual(base, leftmost, const);
}

void const *scc_rbtree_impl_rightmost_value(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    struct scc_rbnode_base const *rightmost = scc_rbtree_rightmost(base->rb_root);
    return scc_rbnode_value_qual(base, rightmost, const);
}

void const *scc_rbtree_impl_successor(void const *iter) {
    struct scc_rbnode_base const *node = scc_rbnode_impl_base_qual(iter, const);
    size_t const offset = (unsigned char const *)iter - (unsigned char const *)node;
    if (scc_rbnode_thread(node, scc_rbdir_right)) {
        node = node->rn_right;
    }
    else {
        node = scc_rbtree_leftmost(node->rn_right);
    }

    return (unsigned char const *)node + offset;
}

void const *scc_rbtree_impl_predecessor(void const *iter) {
    struct scc_rbnode_base const *node = scc_rbnode_impl_base_qual(iter, const);
    size_t const offset = (unsigned char const *)iter - (unsigned char const *)node;
    if (scc_rbnode_thread(node, scc_rbdir_left)) {
        node = node->rn_left;
    }
    else {
        node = scc_rbtree_rightmost(node->rn_left);
    }

    return (unsigned char const *)node + offset;
}

void *scc_rbtree_impl_clone(void const *rbtree, size_t elemsize) {
    struct scc_rbtree_base const *obase = scc_rbtree_impl_base_qual(rbtree, const);
    size_t basesz = (unsigned char const *)rbtree - (unsigned char const *)obase;
    struct scc_rbtree_base *nbase = scc_rbtree_clone_base(obase, elemsize, basesz);
    if (!nbase) {
        return 0;
    }
    if (!obase->rb_size) {
        return (unsigned char *)nbase + basesz;
    }

    /* Know how many nodes are needed */
    if (!scc_arena_reserve(&nbase->rb_arena, obase->rb_size)) {
        free(nbase);
        return 0;
    }

    void *ntree = 0;

    size_t nodesz = nbase->rb_dataoff + elemsize;
    nbase->rb_root = scc_arena_alloc(&nbase->rb_arena);
    scc_memcpy(nbase->rb_root, obase->rb_root, nodesz);
    /* Links updated recursively */
    nbase->rb_root->rn_left = (void *)&nbase->rb_sentinel;
    nbase->rb_root->rn_right = (void *)&nbase->rb_sentinel;

    struct stage {
        struct scc_rbnode_base *old;
        struct scc_rbnode_base **new;
        struct scc_rbnode_base *parent;
        enum scc_rbdir dir;
    };

    scc_deque(struct stage) deque = scc_deque_new(struct stage);
    if (!scc_deque_reserve(&deque, nbase->rb_size)) {
        goto epilogue;
    }

#define push_stage(o, p, d)                             \
    scc_deque_push_back(&deque, (struct stage) {        \
        .old = scc_rbnode_link(o, d),                   \
        .new = &scc_rbnode_link(p, d),                  \
        .parent = p,                                    \
        .dir = d                                        \
    })

    for (int i = 0; i <= scc_rbdir_right; ++i) {
        if (scc_rbnode_thread(obase->rb_root, i)) {
            continue;
        }
        if (!push_stage(obase->rb_root, nbase->rb_root, i)) {
            goto epilogue;
        }
    }

    struct scc_rbnode_base *n;
    while (!scc_deque_empty(deque)) {
        struct stage *s = &scc_deque_front(deque);

        n = scc_arena_alloc(&nbase->rb_arena);
        assert(n);
        assert(s->old);
        scc_memcpy(n, s->old, nodesz);
        scc_rbnode_link(n, s->dir) = *s->new;
        scc_rbnode_link(n, !s->dir) = s->parent;
        *s->new = n;

        if (!scc_rbnode_thread(s->old, scc_rbdir_left)) {
            if (!push_stage(s->old, *s->new, scc_rbdir_left)) {
                goto epilogue;
            }
        }
        if (!scc_rbnode_thread(s->old, scc_rbdir_right)) {
            if (!push_stage(s->old, *s->new, scc_rbdir_right)) {
                goto epilogue;
            }
        }
        (void)scc_deque_pop_front(deque);
    }

    ntree = (unsigned char *)nbase + basesz;

epilogue:
    scc_deque_free(deque);
    if (!ntree) {
        free(nbase);
    }
    return ntree;
}
