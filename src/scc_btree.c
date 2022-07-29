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

//? .. c:function:: void *scc_btnode_value(\
//?     struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node, \
//?     size_t index, size_t elemsize)
//?
//?     Compute and return address of the given node's indexth child
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: B-tree base address
//?     :param node: Node base address
//?     :param index: Index of the child whose address is to be computed
//?     :param elemsize: Size of the elements in the tree
//?     :returns: Address of the indexth child of the given node
static inline void *scc_btnode_value(
    struct scc_btree_base const *restrict base,
    struct scc_btnode_base *restrict node,
    size_t index,
    size_t elemsize
) {
    return (unsigned char *)scc_btnode_data(base, node) + index * elemsize;
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

//? .. c:function:: size_t scc_btnode_emplace_leaf(\
//?        struct scc_btree_base *restrict base, struct scc_btnode_base *restrict node, void *restrict value, size_t elemsize)
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
//?     :returns: The lower bound of the newly inserted element
static size_t scc_btnode_emplace_leaf(
    struct scc_btree_base *restrict base,
    struct scc_btnode_base *restrict node,
    void *restrict value,
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
    return bound;
}

//? .. c:function:: void scc_btnode_emplace(\
//?     struct scc_btree base *restrict base, struct scc_btnode_base *restrict node, \
//?     struct scc_btnode_base *restrict child, void *restrict value, size_t elemsize)
//?
//?     Insert the given value with accompanying child subtree in non-leaf node. The
//?     given node must have at least one vacant slot
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: B-tree base address
//?     :param node: The node to insert the value in
//?     :param child: Root of the subtree to insert
//?     :param value: The value to insert
//?     :param elemsize: Size of the elements in the B-tree
static void scc_btnode_emplace(
    struct scc_btree_base *restrict base,
    struct scc_btnode_base *restrict node,
    struct scc_btnode_base *restrict child,
    void *restrict value,
    size_t elemsize
) {
    size_t bound = scc_btnode_emplace_leaf(base, node, value, elemsize);
    struct scc_btnode_base **links = scc_btnode_links(base, node);
    if(bound < node->bt_nkeys - 1u) {
        memmove(links + bound + 2u, links + bound + 1u, (node->bt_nkeys - bound - 1u) * sizeof(*links));
    }
    links[bound + 1u] = child;
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
    node->bt_nkeys = 0u;
    scc_btnode_flags_clear(node);
}

//? .. c:function:: int scc_btnode_find_linkindex(\
//?     struct scc_btree_base const *restrict base, \
//?     struct scc_btnode_base const *restrict node, \
//?     struct scc_btnode_base *restrict p)
//?
//?     Find and return the index of the given node in the link
//?     array of p.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: B-tree base address
//?     :param node: Base address of the node to be searched for
//?     :param p: Base address of the parent node whose link array is to be searched
//?     :returns: Index of :code:`node` in the link array of :code:`p`, or :code:`-1` if
//?               the node is not found in the array
static inline int scc_btnode_find_linkindex(
    struct scc_btree_base const *restrict base,
    struct scc_btnode_base const *restrict node,
    struct scc_btnode_base *restrict p
) {
    struct scc_btnode_base **plinks = scc_btnode_links(base, p);
    for(int i = 0; i < (int)p->bt_nkeys + 1; ++i) {
        if(plinks[i] == node) {
            return i;
        }
    }
    return -1;
}

//? .. c:function:: struct scc_btnode_base *scc_btnode_split_preemptive(\
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
static struct scc_btnode_base *scc_btnode_split_preemptive(
    struct scc_btree_base *restrict base,
    struct scc_btnode_base *restrict node,
    struct scc_btnode_base *p,
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
        scc_btree_new_root(base, p, node, right);
    }

    unsigned char *ldata = scc_btnode_data(base, node);
    unsigned char *rdata = scc_btnode_data(base, right);

    struct scc_btnode_base **llinks = scc_btnode_links(base, node);
    struct scc_btnode_base **rlinks = scc_btnode_links(base, right);

    node->bt_nkeys >>= 1u;
    right->bt_nkeys = node->bt_nkeys;

    right->bt_flags = node->bt_flags;

    memcpy(rdata, ldata + (node->bt_nkeys + 1u) * elemsize, right->bt_nkeys * elemsize);
    if(!scc_btnode_is_leaf(node)) {
        memcpy(rlinks, llinks + node->bt_nkeys + 1u, (node->bt_nkeys + 1u) * sizeof(*rlinks));
    }

    int bound = scc_btnode_find_linkindex(base, node, p);
    assert(bound != -1);
    assert(bound < base->bt_order);

    unsigned char *pdata = scc_btnode_data(base, p);
    struct scc_btnode_base **plinks = scc_btnode_links(base, p);
    if(bound < p->bt_nkeys) {
        memmove(pdata + (bound + 1u) * elemsize, pdata + bound * elemsize, (p->bt_nkeys - bound) * elemsize);
        memmove(plinks + bound + 1u, plinks + bound, (p->bt_nkeys - bound + 1u) * sizeof(*plinks));
    }
    memcpy(pdata + bound * elemsize, ldata + node->bt_nkeys * elemsize, elemsize);
    plinks[bound + 1u] = right;
    ++p->bt_nkeys;

    return right;
}

//? .. struct scc_btnode_base *scc_btnode_split_non_preemptive(\
//?     struct scc_btree_base *restrict base, struct scc_btnode_base *restrict base, \
//?     struct scc_btnode_base *restrict child, struct scc_btnode_base *p, \
//?     void *restrict value, size_t elemsize)
//?
//?     Split the given node in two, moving values as required. The supplied value is
//?     treated as if inserted at its appropriate position before the split. The middlemost
//?     value to be moved to the parent node is written to the first vacant
//?     slot in the newly allocated right node's data array.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: B-tree base address
//?     :param node: The node to split
//?     :param child: Child link to insert
//?     :param p: Parent of the given node, or NULL if :c:texpr:`node == base->bt_root`
//?     :param value: The value that were to be written to the node, causing the split
//?     :param elemsize: Size of the elements in the tree
//?     :returns: Address of the new node allocated for the split, or
//?               :code:`NULL` on allocation failure
static struct scc_btnode_base *scc_btnode_split_non_preemptive(
    struct scc_btree_base *restrict base,
    struct scc_btnode_base *restrict node,
    struct scc_btnode_base *restrict child,
    struct scc_btnode_base *p,
    void *restrict value,
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
        scc_btree_new_root(base, p, node, right);
    }

    size_t bound = scc_btnode_lower_bound(base, node, value, elemsize);
    assert(bound <= node->bt_nkeys);

    node->bt_nkeys >>= 1u;
    right->bt_nkeys = node->bt_nkeys;
    right->bt_flags = node->bt_flags;

    unsigned char *rdata = scc_btnode_data(base, right);
    struct scc_btnode_base **rlinks = scc_btnode_links(base, right);
    struct scc_btnode_base **llinks = scc_btnode_links(base, node);

    void *nval = scc_btnode_value(base, right, right->bt_nkeys, elemsize);
    if(bound == node->bt_nkeys) {
        memcpy(nval, value, elemsize);
        memcpy(rdata, scc_btnode_value(base, node, node->bt_nkeys, elemsize), right->bt_nkeys * elemsize);
        if(!scc_btnode_is_leaf(node)) {
            *rlinks = child;
            memcpy(rlinks + 1u, llinks + node->bt_nkeys + 1u, right->bt_nkeys * sizeof(*rlinks));
        }
    }
    else if(bound < node->bt_nkeys) {
        memcpy(nval, scc_btnode_value(base, node, node->bt_nkeys - 1u, elemsize), elemsize);
        size_t nmov = node->bt_nkeys - bound - 1u;
        unsigned char *from = scc_btnode_value(base, node, bound, elemsize);
        if(nmov) {
            memmove(from + elemsize, from, nmov * elemsize);
        }
        memcpy(from, value, elemsize);
        memcpy(rdata, scc_btnode_value(base, node, node->bt_nkeys, elemsize), right->bt_nkeys * elemsize);
        if(!scc_btnode_is_leaf(node)) {
            memcpy(rlinks, llinks + node->bt_nkeys, (right->bt_nkeys + 1u) * sizeof(*rlinks));
            if(nmov) {
                memmove(llinks + bound + 2u, llinks + bound + 1u, (node->bt_nkeys - bound) * sizeof(*llinks));
            }
            llinks[bound + 1u] = child;
        }
    }
    else {
        unsigned char *sdata = scc_btnode_value(base, node, node->bt_nkeys, elemsize);
        memcpy(nval, sdata, elemsize);
        sdata += elemsize;
        size_t nbef = bound - node->bt_nkeys - 1u;
        if(nbef) {
            size_t nbytes = nbef * elemsize;
            memcpy(rdata, sdata, nbytes);
            sdata += nbytes;
            rdata += nbytes;
        }
        memcpy(rdata, value, elemsize);

        size_t naft = (node->bt_nkeys << 1u) - bound;
        if(naft) {
            memcpy(rdata + elemsize, sdata, naft * elemsize);
        }

        if(!scc_btnode_is_leaf(node)) {
            ++nbef;
            memcpy(rlinks, llinks + node->bt_nkeys + 1u, nbef * sizeof(*rlinks));
            rlinks[bound - node->bt_nkeys] = child;
            if(naft) {
                ++nbef;
                memcpy(rlinks + 1u, llinks + node->bt_nkeys + nbef, naft * sizeof(*rlinks));
            }
        }
    }

    return right;
}

//? .. c:function:: _Bool scc_btree_insert_preemptive(struct scc_btree_base *base, void *btreeaddr, size_t elemsize)
//?
//?     Insert the element in :c:texpr:`*(void **)btreeaddr` using :ref:`preemptive splitting <preemptive_split>`.
//?     The order of the given tree must be even lest the B-tree invariants be violated.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-tree
//?     :param btreeaddr: Address of the B-tree handle
//?     :param elemsize: Size of the elements in the tree
//?     :returns: :code:`true` if the value was successfully inserted, otherwise :code:`false`.
static _Bool scc_btree_insert_preemptive(struct scc_btree_base *base, void *btreeaddr, size_t elemsize) {
    struct scc_btnode_base *curr = base->bt_root;
    struct scc_btnode_base *p = 0;

    struct scc_btnode_base *right;
    size_t bound;
    while(true) {
        bound = scc_btnode_lower_bound(base, curr, *(void **)btreeaddr, elemsize);
        if(curr->bt_nkeys == base->bt_order - 1u) {
            right = scc_btnode_split_preemptive(base, curr, p, elemsize);
            if(!right) {
                return false;
            }

            if(bound >= curr->bt_nkeys) {
                curr = right;
            }
        }
        if(scc_btnode_is_leaf(curr)) {
            break;
        }

        bound = scc_btnode_lower_bound(base, curr, *(void **)btreeaddr, elemsize);
        p = curr;
        curr = scc_btnode_child(base, curr, bound);
    }

    scc_btnode_emplace_leaf(base, curr, *(void **)btreeaddr, elemsize);
    ++base->bt_size;
    return true;
}

//? .. c:function:: _Bool scc_btree_insert_non_preemptive(struct scc_btree_base *base, void *btreeaddr, size_t elemsize)
//?
//?     Insert the element in :c:texpr:`*(void **)btreeaddr` using :ref:`non-preemptive splitting <non_preemptive_split>`.
//?     Called only for trees whose order is odd
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-tree
//?     :param btreeaddr: Address of the B-tree handle
//?     :param elemsize: Size of the elements in the tree
//?     :returns: :code:`true` if the value was successfully inserted, otherwise :code:`false`.
static _Bool scc_btree_insert_non_preemptive(struct scc_btree_base *base, void *btreeaddr, size_t elemsize) {
    size_t origsz = base->bt_size;
    scc_stack(struct scc_btnode_base *) stack = scc_stack_new(struct scc_btnode_base *);

    /* Root has parent NULL */
    if(!scc_stack_push(&stack, 0)) {
        goto epilogue;
    }

    /* Splitting root requires new root */
    size_t req_allocs = base->bt_root->bt_nkeys == base->bt_order - 1u;

    size_t bound;
    struct scc_btnode_base *curr = base->bt_root;
    while(1) {
        if(curr->bt_nkeys == base->bt_order - 1u) {
            ++req_allocs;
        }
        else {
            req_allocs = 0u;
        }

        if(scc_btnode_is_leaf(curr)) {
            break;
        }

        if(!scc_stack_push(&stack, curr)) {
            goto epilogue;
        }

        bound = scc_btnode_lower_bound(base, curr, *(void **)btreeaddr, elemsize);
        curr = scc_btnode_child(base, curr, bound);
    }

    if(curr->bt_nkeys < base->bt_order - 1u) {
        scc_btnode_emplace_leaf(base, curr, *(void **)btreeaddr, elemsize);
        ++base->bt_size;
        goto epilogue;
    }

    assert(req_allocs);
    /* Make sure tree can't end up in invalid state due to
     * allocation failures */
    if(!scc_arena_reserve(&base->bt_arena, req_allocs)) {
        goto epilogue;
    }

    struct scc_btnode_base *p;
    struct scc_btnode_base *right = 0;
    void *value = *(void **)btreeaddr;
    while(1) {
        p = scc_stack_top(stack);

        right = scc_btnode_split_non_preemptive(base, curr, right, p, value, elemsize);
        /* Cannot fail thanks to reserve */
        assert(right);
        value = scc_btnode_value(base, right, right->bt_nkeys, elemsize);

        curr = p;
        if(!curr || curr->bt_nkeys < base->bt_order - 1u) {
            break;
        }

        scc_stack_pop(stack);
    }

    if(!curr) {
        curr = base->bt_root;
    }

    scc_btnode_emplace(base, curr, right, value, elemsize);
    ++base->bt_size;

epilogue:
    scc_stack_free(stack);
    return origsz < base->bt_size;
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
    if(scc_bits_is_even(base->bt_order)) {
        return scc_btree_insert_preemptive(base, btreeaddr, elemsize);
    }

    return scc_btree_insert_non_preemptive(base, btreeaddr, elemsize);
}

void const *scc_btree_impl_find(void const *btree, size_t elemsize) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);

    struct scc_btnode_base *curr = base->bt_root;

    size_t bound;
    void *addr;
    while(true) {
        bound = scc_btnode_lower_bound(base, curr, btree, elemsize);
        if(bound < curr->bt_nkeys) {
            addr = (unsigned char *)scc_btnode_data(base, curr) + bound * elemsize;
            if(!base->bt_compare(addr, btree)) {
                return addr;
            }
        }

        if(scc_btnode_is_leaf(curr)) {
            break;
        }

        curr = scc_btnode_child(base, curr, bound);
    }

    return 0;
}
