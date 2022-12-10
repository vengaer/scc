#include <scc/mem.h>
#include <scc/scc_rbtree.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#define rb_root rb_sentinel.rs_left

size_t scc_rbtree_impl_npad(void const *rbtree);
size_t scc_rbtree_size(void const *rbtree);
_Bool scc_rbtree_empty(void const *rbtree);
void const *scc_rbtree_impl_iterstop(void const *rbtree);
size_t scc_rbnode_link_offset(struct scc_rbnode_base const *node);
_Bool scc_rbnode_thread(struct scc_rbnode_base const *node, enum scc_rbdir dir);
size_t scc_rbnode_bkoff(void const *iter);

//? .. c:enum:: @flags
//?
//?     Flags used for internal thread tracking
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:enumerator:: SCC_RBLTHRD
//?
//?         Indicates that the left link of a node is a thread
//?
//?     .. c:enumerator:: scc_rbcolor_red
//?
//?         Indicates that the right link of a node is a thread
//?
//?     .. c:enumerator:: SCC_RBLEAF
//?
//?         Indicates that both the left and right links of a node are threads
enum {
    SCC_RBLTHRD = 0x01,
    SCC_RBRTHRD = 0x02,
    SCC_RBLEAF = SCC_RBLTHRD | SCC_RBRTHRD
};

//? .. c:function:: void scc_rbtree_set_bkoff(unsigned char *rbtree, unsigned char bkoff)
//?
//?     Set the :ref:`rb_bkoff <unsigned_char_rb_bkoff>` field to the given value
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param rbtree: Handle to the rbtree
//?     :param bkoff:  The value to set the bkoff field to
static inline void scc_rbtree_set_bkoff(unsigned char *rbtree, unsigned char bkoff) {
    rbtree[-1] = bkoff;
}

//? .. c:function:: void scc_rbnode_set_bkoff(\
//?     struct scc_rbtree_base const *restrict base, struct scc_rbnode_base *restrict node)
//?
//?     Set the :ref:`rn_bkoff <unsigned_char_rn_bkoff>` field of the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the rbtree
//?     :param node: The node whose :ref:`rn_bkoff <unsigned_char_rn_bkoff>` field is to be
//?                  set
static inline void scc_rbnode_set_bkoff(
    struct scc_rbtree_base const *restrict base,
    struct scc_rbnode_base *restrict node
) {
    unsigned const bkoff = base->rb_dataoff - offsetof(struct scc_rbnode_base, rn_data);
    assert(bkoff <= UCHAR_MAX);
    ((unsigned char *)node)[base->rb_dataoff - 1u] = bkoff;
}

//? .. c:function:: void scc_rbnode_set(struct scc_rbnode_base *node, enum scc_rbdir dir)
//?
//?     Set node's thread flag in the given direction
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node whose flag is to be set
//?     :param dir:  The direction of the link whose corresponding flag is to be set
static inline void scc_rbnode_set(struct scc_rbnode_base *node, enum scc_rbdir dir) {
    node->rn_flags |= (1 << dir);
}

//? .. c:function:: void scc_rbnode_unset(struct scc_rbnode_base *node, enum scc_rbdir dir)
//?
//?     Unset node's thread flag in the given direction
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node whose flag is to be unset
//?     :param dir:  The direction of the link whose corresponding flag is to be unset
static inline void scc_rbnode_unset(struct scc_rbnode_base *node, enum scc_rbdir dir) {
    node->rn_flags &= ~(1 << dir);
}

//? .. c:function:: void scc_rbnode_thread_from(struct scc_rbnode_base *restrict dst, \
//?     struct scc_rbnode_base const *restrict src, enum scc_rbdir dir)
//?
//?     Copy thread flag for the given direction from :code:`src` to :code:`dst`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param dst: Node to copy the flag bit to
//?     :param src: Node to copy the flag bit from
//?     :param dir: The direction corresponding to the flag bit
static inline void scc_rbnode_thread_from(
    struct scc_rbnode_base *restrict dst,
    struct scc_rbnode_base const *restrict src,
    enum scc_rbdir dir
) {
    dst->rn_flags = (dst->rn_flags & ~(1 << dir)) | (src->rn_flags & (1 << dir));
}

//? .. c:function:: _Bool scc_rbnode_has_thread_link(struct scc_rbnode_base const *node)
//?
//?     Determine whether the given node has at least one thread link
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node whose links are to be checked
//?     :returns:    :code:`true` if the given node has at least one thread link,
//?                  otherwise :code:`false`
static inline _Bool scc_rbnode_has_thread_link(struct scc_rbnode_base const *node) {
    return scc_rbnode_thread(node, scc_rbdir_left) ||
           scc_rbnode_thread(node, scc_rbdir_right);
}

//? .. c:function:: _Bool scc_rbnode_red(struct scc_rbnode_base const *node)
//?
//?     Determine whether the given node is red
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node to check
//?     :returns: :code:`true` if the given node is red, otherwise :code:`false`.
static inline _Bool scc_rbnode_red(struct scc_rbnode_base const *node) {
    return node->rn_color == scc_rbcolor_red;
}

//? .. c:function:: _Bool scc_rbnode_red_safe(struct scc_rbnode_base const *node, enum scc_rbdir dir)
//?
//?     Check if the given node has a red child in the given direction
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node:   The node whose child is to be checked
//?     :param dir:    The direction of the child to check
//?     :returns:      A :code:`_Bool` signalling whether the child in the given directory
//?                    is red
//?     :retval true:  The given node has a child in the given direction and the child in
//?                    question is red
//?     :retval false: The given node has no child in the given direction
//?     :retval false: The given node has a black child in the given direction
static inline _Bool scc_rbnode_red_safe(struct scc_rbnode_base const *node, enum scc_rbdir dir) {
    return !scc_rbnode_thread(node, dir) &&
            scc_rbnode_red(scc_rbnode_link_qual(node, dir, const));
}

//? .. c:function:: void *scc_rbnode_children_red_safe(struct scc_rbnode_base const *node)
//?
//?     Check if the given node has two red children
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node whose children is to be checked
//?     :returns: A :code:`_Bool` indicating whether both the node's children are red
//?     :retval true: The node has two children and both are red
//?     :retval false: The node does not have two children
//?     :retval false: The node has two children but at least one of them is black
static inline _Bool scc_rbnode_children_red_safe(struct scc_rbnode_base const *node) {
    return scc_rbnode_red_safe(node, scc_rbdir_left) &&
           scc_rbnode_red_safe(node, scc_rbdir_right);
}

//? .. c:function:: _Bool scc_rbnode_has_red_child(struct scc_rbnode_base const *node)
//?
//?     Check if the given node has at least one red child
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node:   The node whose children are to be examined
//?     :returns:      A :code:`_Bool` indicating whether the node has any red children
//?     :retval true:  The node has at least one red child
//?     :retval false: The node has no children
//?     :retval false: The node has at least one child but none of them are red
static inline _Bool scc_rbnode_has_red_child(struct scc_rbnode_base const *node) {
    return scc_rbnode_red_safe(node, scc_rbdir_left) ||
           scc_rbnode_red_safe(node, scc_rbdir_right);
}

//? .. c:function:: void scc_rbnode_mkblack(struct scc_rbnode_base *node)
//?
//?     Color the given node black
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node to color
static inline void scc_rbnode_mkblack(struct scc_rbnode_base *node) {
    node->rn_color = scc_rbcolor_black;
}

//? .. c:function:: void scc_rbnode_mkred(struct scc_rbnode_base *node)
//?
//?     Color the given node red
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node to color
static inline void scc_rbnode_mkred(struct scc_rbnode_base *node) {
    node->rn_color = scc_rbcolor_red;
}

//? .. c:function:: void scc_rbnode_mkleaf(struct scc_rbnode_base *node)
//?
//?     Mark the given node as a leaf
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: The node to mark
static inline void scc_rbnode_mkleaf(struct scc_rbnode_base *node) {
    node->rn_flags = SCC_RBLEAF;
}

//? .. c:function:: int scc_rbtree_compare(\
//?     struct scc_rbtree_base const *restrict base, \
//?     struct scc_rbnode_base const *restrict node, \
//?     void const *restrict value)
//?
//?     Convenience wrapper for invoking :c:expr:`base->rb_compare` on the
//?     value in the given node and the value parameter
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base:  Base address of the rbtree
//?     :param node:  The node whose value is to be used in the comparison
//?     :param value: The value to compare to the value in the given node
//?     :returns:     An integer ordering the two values. See :ref:`scc_rbcompare <scc_rbcompare>`
//?                   for details.
static inline int scc_rbtree_compare(
    struct scc_rbtree_base const *restrict base,
    struct scc_rbnode_base const *restrict node,
    void const *restrict value
) {
    return base->rb_compare(scc_rbnode_value_qual(base, node, const), value);
}

//? .. c:function:: struct scc_rbnode_base *scc_rbtree_rotate_single(struct scc_rbnode_base *root, enum scc_rbdir dir)
//?
//?     Perform single rotation of subtree anchored at the given root
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param root: The root around which the rotation is to be performed
//?     :param dir:  The direction to rotate in
//?     :returns:    The new root of the subtree
static struct scc_rbnode_base *scc_rbtree_rotate_single(struct scc_rbnode_base *root, enum scc_rbdir dir) {
    struct scc_rbnode_base *n = scc_rbnode_link(root, !dir);

    if(scc_rbnode_thread(n, dir)) {
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

//? .. c:function:: struct scc_rbnode_base *scc_rbtree_rotate_double(struct scc_rbnode_base *root, enum scc_rbdir dir)
//?
//?     Perform double rotation of subtree anchored at the given root
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param root: The root around which the rotation is to be performed
//?     :param dir:  The direction to rotate in
//?     :returns:    The new root of the subtree
static inline struct scc_rbnode_base *scc_rbtree_rotate_double(struct scc_rbnode_base *root, enum scc_rbdir dir) {
    scc_rbnode_link(root, !dir) = scc_rbtree_rotate_single(scc_rbnode_link(root, !dir), !dir);
    return scc_rbtree_rotate_single(root, dir);
}

//? .. c:function:: void scc_rbtree_balance_insertion(\
//?     struct scc_rbnode_base *n, struct scc_rbnode_base *p, \
//?     struct scc_rbnode_base *gp, struct scc_rbnode_base *ggp)
//?
//?     Preemptively balance the section around n, p and gp for insertion.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param n:   The bottommost node of the secion
//?     :param p:   Parent node of n
//?     :param gp:  Parent node of p
//?     :param ggp: Parent node of gp
static void scc_rbtree_balance_insertion(
    struct scc_rbnode_base *n,
    struct scc_rbnode_base *p,
    struct scc_rbnode_base *gp,
    struct scc_rbnode_base *ggp
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

//? .. c:function:: struct scc_rbnode_base *scc_rbtree_balance_removal(\
//?     struct scc_rbnode_base *n, struct scc_rbnode_base *p, \
//?     struct scc_rbnode_base *gp, enum scc_rbdir dir)
//?
//?     Preemptively balance the section around n, p and gp for removal.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param n:   The bottommost node of the section
//?     :param p:   Parent node of n
//?     :param gp:  Parent node of p
//?     :param dir: Direction of the link to be traversed next
//?     :returns:   The new root of the subtree after balancing
static struct scc_rbnode_base *scc_rbtree_balance_removal(
    struct scc_rbnode_base *n,
    struct scc_rbnode_base *p,
    struct scc_rbnode_base *gp,
    enum scc_rbdir dir
) {
    enum scc_rbdir pdir = p->rn_right == n;
    enum scc_rbdir gpdir = gp->rn_right == p;

    if(scc_rbnode_red_safe(n, !dir)) {
        scc_rbnode_link(p, pdir) = scc_rbtree_rotate_single(n, dir);
        return scc_rbnode_link(p, pdir);
    }

    if(!scc_rbnode_thread(p, !pdir)) {
        struct scc_rbnode_base *sibling = scc_rbnode_link(p, !pdir);
        if(scc_rbnode_has_red_child(sibling)) {
            if(scc_rbnode_red_safe(sibling, pdir)) {
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

//? .. c:function:: struct scc_rbnode_base *scc_rbnode_new(\
//?     struct scc_rbtree_base *restrict base,\
//?     void const *restrict value,\
//?     size_t elemsize)
//?
//?     Allocate and initialize a new rbnode
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base:     Base address of the rbtree
//?     :param value:    Value to be assigned to the node
//?     :param elemsize: Size of the elements in the tree
//?     :returns:        Address of a newly allocated node in the arena, or :code:`NULL` on allocation failure
static inline struct scc_rbnode_base *scc_rbnode_new(
    struct scc_rbtree_base *restrict base,
    void const *restrict value,
    size_t elemsize
) {
    struct scc_rbnode_base *node = scc_arena_alloc(&base->rb_arena);
    if(!node) {
        return 0;
    }
    memcpy(scc_rbnode_value(base, node), value, elemsize);
    scc_rbnode_set_bkoff(base, node);
    return node;
}

//? .. c:function:: struct scc_rbnode_base const *scc_rbtree_leftmost(\
//?     struct scc_rbnode_base const *root)
//?
//?     Find the leftmost node in the subtree spanning from the given root
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param root: The root of the subtree
//?     :returns:    Address of the leftmost node in the subtree
static inline struct scc_rbnode_base const *scc_rbtree_leftmost(struct scc_rbnode_base const *root) {
    while(!scc_rbnode_thread(root, scc_rbdir_left)) {
        root = root->rn_left;
    }
    return root;
}

//? .. c:function:: struct scc_rbnode_base const *scc_rbtree_rightmost(\
//?     struct scc_rbnode_base const *root)
//?
//?     Find the rightmost node in the subtree spanning from the given root
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param root: The root of the subtree
//?     :returns:    Address of the rightmost node in the subtree
static inline struct scc_rbnode_base const *scc_rbtree_rightmost(struct scc_rbnode_base const *root) {
    while(!scc_rbnode_thread(root, scc_rbdir_right)) {
        root = root->rn_right;
    }
    return root;
}

//? .. c:function:: _Bool scc_rbtree_insert_empty(struct scc_rbtree_base *restrict base, void *restrict handle, size_t elemsize)
//?
//?     Insert the value at :c:expr:`*handle` into the empty rbtree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the rbtree
//?     :param handle: rbtree handle
//?     :param elemsize: Size of the elements in the tree
//?     :returns: :code:`true` if the value was inserted, :code:`false` on allocation failure
static inline _Bool scc_rbtree_insert_empty(struct scc_rbtree_base *restrict base, void *restrict handle, size_t elemsize) {
    struct scc_rbnode_base *node = scc_rbnode_new(base, handle, elemsize);
    if(!node) {
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

//? .. c:function:: _Bool scc_rbtree_insert_nonempty(struct scc_rbtree_base *restrict base, void const *restrict handle, size_t elemsize)
//?
//?     Insert the value at :c:expr:`*handle` inte the non-empty rbtree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the rbtree
//?     :param handle: rbtree handle
//?     :param elemsize: Size of the elements in the tree
//?     :returns: :code:`true` if the vlaue was inserted, otherwise :code:`false`.
//?     :retval true: The value was inserted
//?     :retval false: The value was already present in the tree
//?     :retval false: Allocation failure
static _Bool scc_rbtree_insert_nonempty(struct scc_rbtree_base *restrict base, void const *restrict handle, size_t elemsize) {
    struct scc_rbnode_base *n = base->rb_root;
    struct scc_rbnode_base *p = (void *)&base->rb_sentinel;
    struct scc_rbnode_base *gp = &(struct scc_rbnode_base) { .rn_left = p };
    struct scc_rbnode_base *ggp = &(struct scc_rbnode_base) { .rn_left = gp };

    enum scc_rbdir dir;
    int rel;

    while(1) {
        if(scc_rbnode_children_red_safe(n)) {
            /* Push red coloring towards root */
            scc_rbtree_balance_insertion(n, p, gp, ggp);
        }
        rel = scc_rbtree_compare(base, n, handle);
        if(!rel) {
            /* Already in tree */
            scc_rbnode_mkblack(base->rb_root);
            return false;
        }
        dir = rel <= 0;
        if(scc_rbnode_thread(n, dir)) {
            break;
        }

        ggp = gp;
        gp = p;
        p = n;
        n = scc_rbnode_link(n, dir);
    }

    /* Allocate */
    struct scc_rbnode_base *new = scc_rbnode_new(base, handle, elemsize);
    if(!new) {
        scc_rbnode_mkblack(base->rb_root);
        return false;
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
    return true;
}

void *scc_rbtree_impl_new(void *base, size_t coff) {
#define base ((struct scc_rbtree_base *)base)
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
#undef base
}

void scc_rbtree_free(void *rbtree) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(rbtree);
    scc_arena_release(&base->rb_arena);
}

_Bool scc_rbtree_impl_insert(void *rbtreeaddr, size_t elemsize) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(*(void **)rbtreeaddr);
    if(!base->rb_size) {
        return scc_rbtree_insert_empty(base, *(void **)rbtreeaddr, elemsize);
    }

    return scc_rbtree_insert_nonempty(base, *(void **)rbtreeaddr, elemsize);
}

void const *scc_rbtree_impl_find(void const *rbtree) {
    struct scc_rbtree_base const *base = scc_rbtree_impl_base_qual(rbtree, const);
    struct scc_rbnode_base const *p = (void const *)&base->rb_sentinel;
    struct scc_rbnode_base *n = base->rb_root;

    enum scc_rbdir dir = scc_rbdir_left;
    int rel;

    while(!scc_rbnode_thread(p, dir)) {
        rel = scc_rbtree_compare(base, n, rbtree);
        if(!rel) {
            return scc_rbnode_value_qual(base, n, const);
        }

        dir = rel <= 0;
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

    while(!scc_rbnode_thread(p, dir)) {
        rel = scc_rbtree_compare(base, n, rbtree);
        if(!rel) {
            found = n;
        }

        dir = rel <= 0;

        if(!scc_rbnode_red(n) && !scc_rbnode_red_safe(n, dir)) {
            p = scc_rbtree_balance_removal(n, p, gp, dir);
        }

        gp = p;
        p = n;
        n = scc_rbnode_link(n, dir);
    }

    if(found) {
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
    if(scc_rbnode_thread(node, scc_rbdir_right)) {
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
    if(scc_rbnode_thread(node, scc_rbdir_left)) {
        node = node->rn_left;
    }
    else {
        node = scc_rbtree_rightmost(node->rn_left);
    }

    return (unsigned char const *)node + offset;
}
