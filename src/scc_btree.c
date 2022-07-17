#include <scc/scc_algorithm.h>
#include <scc/scc_btree.h>
#include <scc/scc_stack.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

void *scc_btree_impl_with_order(void *base, size_t coff, size_t rootoff);
size_t scc_btree_impl_npad(void const *btree);
size_t scc_btree_order(void const *btree);
size_t scc_btree_size(void const *btree);

//? .. c:enumerator:: SCC_BTREE_FLAG_LEAF
//?
//?     Bit indicating that a node is leaf
enum { SCC_BTREE_FLAG_LEAF = 0x01 };

//? .. c:function:: void scc_btree_set_bkoff(void *btree, unsigned char bkoff)
//?
//?     Set the :ref:`bt_bkoff <unsigned_char_bt_bkoff>` field of the B-tree
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btree: B-tree handle
//?     :param bkoff: The value to set the :code:`bt_bkoff` field to
static inline void scc_btree_set_bkoff(void *btree, unsigned char bkoff) {
    ((unsigned char *)btree)[-1] = bkoff;
}

//? .. c:function:: void scc_btree_root_init(struct scc_btree_base *base, void *root)
//?
//?     Initialize the root node of the B-tree whose base address is given.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-tree
//?     :param root: Address of the root node
static inline void scc_btree_root_init(struct scc_btree_base *base, void *root) {
    base->bt_root = root;
    base->bt_root->bt_flags |= SCC_BTREE_FLAG_LEAF;
}

//? .. c:function:: _Bool scc_btnode_is_leaf(struct scc_btnode_base const *node)
//?
//?     Determine if the given node is a leaf
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: Pointer to the node to check
//?     :returns: :code:`true` is the node is a leaf, otherwise :code:`false`.
static inline _Bool scc_btnode_is_leaf(struct scc_btnode_base *node) {
    return node->bt_flags & SCC_BTREE_FLAG_LEAF;
}

//? .. c:function:: void scc_btnode_flags_clear(struct scc_btnode_base *node)
//?
//?     Clear the flags of the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: Base address of the node
static inline void scc_btnode_flags_clear(struct scc_btnode_base *node) {
    node->bt_flags = 0u;
}

//? .. c:function:: struct scc_btnode_base *scc_btnode_links(\
//?     struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node)
//?
//?     Compute address of the first element in the :ref:`bt_links <struct_scc_btnode_base_bt_links>` field
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: B-tree base address
//?     :param node: Node base address
//?     :returns: Address of the first element in the :code:`bt_links` field of the given node
static inline struct scc_btnode_base **scc_btnode_links(struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node) {
    return (void *)((unsigned char *)node + base->bt_linkoff);
}

//? .. c:function:: struct scc_btnode_base *scc_btnode_child(\
//?        struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node, size_t index)
//?
//?     Compute address of the indexth child of the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the owning B-tree
//?     :param node: Address of the current node
//?     :param index: Index of the child to be returned
//?     :returns: Address of the indexth child of the given node
static inline struct scc_btnode_base *scc_btnode_child(struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node, size_t index) {
    return scc_btnode_links(base, node)[index];
}


//? .. c:function:: void *scc_btnode_data(struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node)
//?
//?     Compute address of the data array of the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: B-tree base address
//?     :param node: Node base address
//?     :returns: Address of the data array in the given node
static inline void *scc_btnode_data(struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node) {
    return (unsigned char *)node + base->bt_dataoff;
}

//? .. _scc_btnode_lower_bound:
//? .. c:function:: size_t scc_btnode_lower_bound(\
//?        struct scc_btree_base const *restrict base, \
//?        struct scc_btnode_base const *restrict node, \
//?        void const *restrict value, \
//?        size_t elemsize)
//?
//?     Compute the lower bound given value in the supplied node.
//?
//?     .. seealso::
//?
//?         :ref:`scc_algo_lower_bound`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-tree
//?     :param node: Address of the node currently searched
//?     :param value: Value that is probed for
//?     :param elemsize: Size of each element in the node
//?     :returns: The lower bound of :code:`value` in the element array of :code:`node`.
static inline size_t scc_btnode_lower_bound(
    struct scc_btree_base const *base,
    struct scc_btnode_base *node,
    void const *restrict value,
    size_t elemsize
) {
    return scc_algo_lower_bound(value, scc_btnode_data(base, node), node->bt_nkeys, elemsize, base->bt_compare);
}

//? .. c:function:: void scc_btnode_emplace(\
//?        struct scc_btree_base *restrict base, struct scc_btnode_base *restrict node, void *value, size_t elemsize)
//?
//?     Insert the given value in the specified leaf node. The node must
//?     have at least one vacant slot
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: B-tree base address
//?     :param node: Node base address
//?     :param value: Address of the value to insert
//?     :param elemsize: Size of the elements in the node
static void scc_btnode_emplace(
    struct scc_btree_base *restrict base,
    struct scc_btnode_base *restrict node,
    void *value,
    size_t elemsize
) {
    size_t bound = scc_btnode_lower_bound(base, node, value, elemsize);
    size_t off = bound * elemsize;
    unsigned char *data = scc_btnode_data(base, node);
    if(bound < node->bt_nkeys) {
        memmove(data + off + elemsize, data + off, (node->bt_nkeys - bound) * elemsize);
    }
    memcpy(data + off, value, elemsize);
    ++node->bt_nkeys;
}

//? .. c:function:: void scc_btree_new_root(\
//?     struct scc_btree_base const *restrict base, \
//?     struct scc_btnode_base *restrict node, \
//?     struct scc_btnode_base *restrict left, \
//?     struct scc_btnode_base *restrict right)
//?
//?     Initialize the node parameter as if it were the new node of the
//?     given B-tree.
//?
//?     As splitting the previous root always yields a new root with a
//?     single value, the new root always has exactly two links.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-tree
//?     :param node: The node to be prepared as new root
//?     :param left: Root of the left subtree
//?     :param right: Root of the right subtree
static inline void scc_btree_new_root(
    struct scc_btree_base const *restrict base,
    struct scc_btnode_base *restrict node,
    struct scc_btnode_base *restrict left,
    struct scc_btnode_base *restrict right
) {
    struct scc_btnode_base **links = scc_btnode_links(base, node);
    links[0] = left;
    links[1] = right;
    node->bt_nkeys = 1u;
    scc_btnode_flags_clear(node);
}

//? .. c:function:: struct scc_btnode_base *scc_btnode_split(\
//?     struct scc_btree_base *restrict base, \
//?     struct scc_btnode_base *restrict node, \
//?     struct scc_btnode_base *p, \
//?     size_t elemsize)
//?
//?     Split the given node in two, moving values and links
//?     as required. The given node is kept as the left child
//?     of p to avoid unnecessary writes.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-tree
//?     :param node: Base address of the node to split
//?     :param p: Pointer to node's parent. Should node be the
//?               root, this pointer is NULL and a new root
//?               is allocated from the arena
//?     :param elemsize: Size of the elements in the B-tree
//?     :returns: Address of the new node allocated for the split, or
//?               :code:`NULL` on allocation failure
static struct scc_btnode_base *scc_btnode_split(
    struct scc_btree_base *restrict base,
    struct scc_btnode_base *restrict node,
    struct scc_btnode_base * p,
    size_t elemsize
) {
    struct scc_btnode_base *right = scc_arena_alloc(&base->bt_arena);
    if(!right) {
        return 0;
    }
    if(!p) {
        p = scc_arena_alloc(&base->bt_arena);
        if(!p) {
            scc_arena_free(&base->bt_arena, right);
            return 0;
        }
        base->bt_root = p;
    }

    unsigned char *ldata = scc_btnode_data(base, node);
    struct scc_btnode_base **llinks = scc_btnode_links(base, node);

    unsigned char *rdata = scc_btnode_data(base, right);
    struct scc_btnode_base **rlinks = scc_btnode_links(base, right);

    node->bt_nkeys >>= 1u;
    right->bt_nkeys = node->bt_nkeys;

    right->bt_flags = node->bt_flags;

    memcpy(rdata, ldata + (node->bt_nkeys + 1u) * elemsize, right->bt_nkeys * elemsize);
    if(!scc_btnode_is_leaf(node)) {
        memcpy(rlinks, llinks + node->bt_nkeys + 1u, (node->bt_nkeys + 1u) * sizeof(*rlinks));
    }

    void const *rootval = ldata + node->bt_nkeys * elemsize;

    if(p == base->bt_root) {
        scc_btree_new_root(base, p, node, right);
        memcpy(scc_btnode_data(base, p), rootval, elemsize);
    }
    else {
        size_t bound = scc_btnode_lower_bound(base, p, rootval, elemsize);
        assert(bound < base->bt_order);
        unsigned char *pdata = scc_btnode_data(base, p);
        struct scc_btnode_base **plinks = scc_btnode_links(base, p);
        if(bound < p->bt_nkeys) {
            memmove(pdata + (bound + 1u) * elemsize, pdata + bound * elemsize, (p->bt_nkeys - bound) * elemsize);
            memmove(plinks + bound + 1u, plinks + bound, (p->bt_nkeys - bound + 1u) * sizeof(*plinks));
        }
        memcpy(pdata + bound * elemsize, rootval, elemsize);
        plinks[bound + 1u] = right;
        ++p->bt_nkeys;
    }

    return right;
}

void *scc_btree_impl_new(void *base, size_t coff, size_t rootoff) {
#define base ((struct scc_btree_base *)base)
    size_t fwoff = coff - offsetof(struct scc_btree_base, bt_fwoff) - sizeof(base->bt_fwoff);
    assert(fwoff <= UCHAR_MAX);
    base->bt_fwoff = (unsigned char)fwoff;
    scc_btree_root_init(base, (unsigned char *)base + rootoff);
    unsigned char *btree = (unsigned char *)base + coff;
    scc_btree_set_bkoff(btree, fwoff);
    return btree;
#undef base
}

void scc_btree_free(void *btree) {
    struct scc_btree_base *base = scc_btree_impl_base(btree);
    scc_arena_release(&base->bt_arena);
}

_Bool scc_btree_impl_insert(void *btreeaddr, size_t elemsize) {
    struct scc_btree_base *base = scc_btree_impl_base(*(void **)btreeaddr);

    struct scc_btnode_base *curr = base->bt_root;
    struct scc_btnode_base *p = 0;

    struct scc_btnode_base *right;
    size_t bound = scc_btnode_lower_bound(base, curr, *(void **)btreeaddr, elemsize);

    if(curr->bt_nkeys == base->bt_order - 1u) {
        right = scc_btnode_split(base, curr, p, elemsize);
        if(!right) {
            return false;
        }

        if(bound >= curr->bt_nkeys) {
            curr = right;
        }
    }

    while(!scc_btnode_is_leaf(curr)) {
        bound = scc_btnode_lower_bound(base, curr, *(void **)btreeaddr, elemsize);
        curr = scc_btnode_child(base, curr, bound);

        if(curr->bt_nkeys == base->bt_order - 1u) {
            right = scc_btnode_split(base, curr, p, elemsize);
            if(!right) {
                return false;
            }

            if(bound >= curr->bt_nkeys) {
                curr = right;
            }
        }
    }

    scc_btnode_emplace(base, curr, *(void **)btreeaddr, elemsize);
    ++base->bt_size;
    return true;
}
