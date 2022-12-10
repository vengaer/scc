#include <scc/algorithm.h>
#include <scc/bits.h>
#include <scc/btmap.h>
#include <scc/stack.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

size_t scc_btmap_order(void const *btmap);
void *scc_btmap_impl_with_order(void *base, size_t coff, size_t rootoff);
size_t scc_btmap_impl_npad(void const *btmap);
size_t scc_btmap_size(void const *btmap);

//? .. c:macro:: BOUND_MASK
//?
//?     Mask to apply to values returned by
//?     :ref:`scc_btmnode_lower_bound <scc_btmnode_lower_bound>`
//?     to compute the actual bound
#define BOUND_MASK ((~(size_t)0u) >> 1u)

//? .. c:enumerator:: SCC_BTMAP_FLAG_LEAF
//?
//?     Bit indicating that a node is leaf
enum { SCC_BTMAP_FLAG_LEAF = 0x01 };

//? .. c:function:: void scc_btmap_set_bkoff(void *btmap, unsigned char bkoff)
//?
//?     Set the :ref:`btm_bkoff <unsigned_char_btm_bkoff>` field of the B-treemap
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param btree: B-treemap handle
//?     :param bkoff: The value to set the :code:`btm_bkoff` field to
static inline void scc_btmap_set_bkoff(void *btmap, unsigned char bkoff) {
    ((unsigned char *)btmap)[-1] = bkoff;
}

//? .. c:function:: void scc_btmap_root_init(struct scc_btmap_base *base, void *root)
//?
//?     Initialize the root node of the B-tree whose base address is given.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the B-treemap
//?     :param root: Address of the root node
static inline void scc_btmap_root_init(struct scc_btmap_base *base, void *root) {
    base->btm_root = root;
    base->btm_root->btm_flags |= SCC_BTMAP_FLAG_LEAF;
}

//? .. c:function:: _Bool scc_btmnode_is_leaf(struct scc_btmnode_base const *node)
//?
//?     Determine if the given node is a leaf
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: Base address of the node to check
//?     :returns: :code:`true` if the node is a leaf, otherwise :code:`false`.
static inline _Bool scc_btmnode_is_leaf(struct scc_btmnode_base const *node) {
    return node->btm_flags & SCC_BTMAP_FLAG_LEAF;
}

//? .. c:function:: void scc_btmnode_flags_clear(struct scc_btmnode_base *node)
//?
//?     Clear the flags f the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param node: Base address of the node
static inline void scc_btmnode_flags_clear(struct scc_btmnode_base *node) {
    node->btm_flags = 0;
}

//? .. c:function:: void *scc_btmnode_keys(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node)
//?
//?     Compute address of the key array in the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: btmap base address
//?     :param node: Node base address
//?     :returns: Addres of the key array in the given node
static inline void *scc_btmnode_keys(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node) {
    return (unsigned char *)node + base->btm_keyoff;
}

//? .. c:function:: void *scc_btmnode_key(\
//?     struct scc_btmap_base const *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     size_t n)
//?
//?     Compute the address of the nth key in the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: Base address of the node
//?     :param n: Index of the key whose address is to be returned
static inline void *scc_btmnode_key(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node, size_t n) {
    return ((unsigned char *)scc_btmnode_keys(base, node)) + n * base->btm_keysize;
}

//? .. c:function:: void *scc_btmnode_vals(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node)
//?
//?     Compute address of the value array in the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: btmap base address
//?     :param node: Node base address
//?     :returns: Address of the value array in the given node
static inline void *scc_btmnode_vals(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node) {
    return (unsigned char *)node + base->btm_valoff;
}

//? .. c:function:: void *scc_btmnode_value(\
//?     struct scc_btmap_base const *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     size_t n)
//?
//?     Compute the address of the nth value in the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: Base address of the node
//?     :param n: Index of the value whose address is to be returned
static inline void *scc_btmnode_value(
    struct scc_btmap_base const *restrict base,
    struct scc_btmnode_base *restrict node,
    size_t n
) {
    return ((unsigned char *)scc_btmnode_vals(base, node)) + n * base->btm_valsize;
}

//? .. c:function:: struct scc_btmnode_base **scc_btmnode_links(\
//?     struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node)
//?
//?     Compute address of the first element in the :ref:`btm_links <struct_scc_btmnode_base_btm_links>` field
//?     in the given node.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: btmap base address
//?     :param node: Nod ebase address
//?     :returns: Address of the first element in the :code:`btm_links` field of the given node
static inline struct scc_btmnode_base **scc_btmnode_links(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node) {
    return (void *)((unsigned char *)node + base->btm_linkoff);
}

//? .. c:function:: struct scc_btmnode_base *scc_btmnode_child(\
//?     struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node, size_t n)
//?
//?     Compute address of the nth child of the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: Base address of the current node
//?     :param n: Index of the child to be returned
//?     :returns: Address of the nth child of the given node
static inline struct scc_btmnode_base *scc_btmnode_child(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node, size_t n) {
    return scc_btmnode_links(base, node)[n];
}

//? .. c:function:: void scc_btmap_new_root(\
//?     struct scc_btmap_base const *restrict base, \
//?     struct scc_btnode_base *restrict node, \
//?     struct scc_btnode_base *restrict left, \
//?     struct scc_btnode_base *restrict right)
//?
//?     Initialize the node parameter as if it were the new node of the
//?     given ``btmap``
//?
//?     As splitting the previous root always yields a new root with a
//?     single value, the new root always has exactly two links.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node: The node to be prepared as new root
//?     :param left: Root of the left subtree
//?     :param right: Root of the right subtree
static inline void scc_btmap_new_root(
    struct scc_btmap_base const *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict left,
    struct scc_btmnode_base *restrict right
) {
    struct scc_btmnode_base **links = scc_btmnode_links(base, node);
    links[0] = left;
    links[1] = right;
    node->btm_nkeys = 0u;
    scc_btmnode_flags_clear(node);
}

//? .. _scc_btmnode_lower_bound:
//? .. c:function:: size_t scc_btmnode_lower_bound(\
//?     struct scc_btmap_base const *base,
//?     struct scc_btmnode_base *node,
//?     void const *restrict value)
//?
//?     Compute the lower bound given the key in the supplied node.
//?
//?     .. seealso::
//?
//?         :ref:`scc_algo_lower_bound`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: Address of the node currently searched
//?     :param value: Value that is probed for
//?     :returns: The lower bound of :code:`value` in the key array of :code:`node`.
static inline size_t scc_btmnode_lower_bound(struct scc_btmap_base const *base, struct scc_btmnode_base *node, void const *restrict value) {
    if(!node->btm_nkeys) {
        return 0u;
    }
    return scc_algo_lower_bound_eq(value, scc_btmnode_keys(base, node), node->btm_nkeys, base->btm_keysize, base->btm_compare);
}

//? .. c:function:: _Bool scc_btmnode_bound_is_eq(size_t bound)
//?
//?     Determine whether the lower bound as computed by
//?     :ref:`scc_btmnode_lower_bound <scc_btmnode_lower_bound>`
//?     indicates that the element at the index compares equal
//?     to the key
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param bound: The computed bound
//?     :returns: :code:`true` if the MSB in :code:`bound` is set,
//?               otherwise :code:`false`.
static inline _Bool scc_btmnode_keyeq(size_t bound) {
    return bound & ~BOUND_MASK;
}

//? .. c:function:: void scc_btmnode_replace_value(\
//?     struct scc_btmap_base const *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     void const *restrict kvpair, \
//?     size_t bound)
//?
//?     Replace the value at the given bound with that in :code:`kvpair`
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the map
//?     :param node: The node to replace the value in
//?     :param kvpair: Key-value pair from whilch the replacement value
//?                    is to be taken
//?     :param bound: Index to insert the value at
static inline void scc_btmnode_replace_value(
    struct scc_btmap_base const *restrict base,
    struct scc_btmnode_base *restrict node,
    void const *restrict kvpair,
    size_t bound
) {
    void *val = scc_btmnode_value(base, node, bound);
    memcpy(val, (unsigned char const *)kvpair + base->btm_kvoff, base->btm_valsize);
}

//? .. c:function:: void scc_btmnode_emplace_leaf(\
//?     struct scc_btmap_base *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     void *restrict kvpair)
//?
//?     Insert the given value in the specified leaf node. The node must
//?     have at least one vacant slot
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: Base address of the node
//?     :param value: Address of the key-value pair to insert
//?     :returns: The lower bound of the newly inserted element
static void scc_btmnode_emplace_leaf(struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, void *restrict kvpair) {
    size_t bound = scc_btmnode_lower_bound(base, node, kvpair);
    if(scc_btmnode_keyeq(bound)) {
        scc_btmnode_replace_value(base, node, kvpair, bound & BOUND_MASK);
        return;
    }

    unsigned char *keys = scc_btmnode_keys(base, node);
    unsigned char *vals = scc_btmnode_vals(base, node);

    size_t keyoff = bound * base->btm_keysize;
    size_t valoff = bound * base->btm_valsize;

    if(bound < node->btm_nkeys) {
        memmove(keys + keyoff + base->btm_keysize, keys + keyoff, (node->btm_nkeys - bound) * base->btm_keysize);
        memmove(vals + valoff + base->btm_valsize, vals + valoff, (node->btm_nkeys - bound) * base->btm_valsize);
    }

    memcpy(keys + keyoff, kvpair, base->btm_keysize);
    memcpy(vals + valoff, (unsigned char *)kvpair + base->btm_kvoff, base->btm_valsize);
    ++node->btm_nkeys;
    ++base->btm_size;
}

//? .. c:function:: void scc_btmnode_emplace(\
//?     struct scc_btmap_base *restrict base, \
//?     struct scc_btmnode_base *restrict node,\
//?     struct scc_btmnode_base *restrict child, \
//?     void *restrict key, \
//?     void *restrict value)
//?
//?     Insert the given key and value with accompanying child
//?     subtree in non-leaf node. The given node must have at
//?     least on vacant slot in its link array
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: The node to insert the value in
//?     :param child: Root of the subtree to insert
//?     :param key: The key to insert
//?     :param value: The value corresponding to the given key
static void scc_btmnode_emplace(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict child,
    void *restrict key,
    void *restrict value
) {
    size_t bound = scc_btmnode_lower_bound(base, node, key);

    /* Tree already traversed, no way the eq bit is set */
    assert(bound <= node->btm_nkeys);

    unsigned char *keys = scc_btmnode_keys(base, node);
    unsigned char *vals = scc_btmnode_vals(base, node);

    size_t keyoff = bound * base->btm_keysize;
    size_t valoff = bound * base->btm_valsize;

    if(bound < node->btm_nkeys) {
        memmove(keys + keyoff + base->btm_keysize, keys + keyoff, (node->btm_nkeys - bound) * base->btm_keysize);
        memmove(vals + valoff + base->btm_valsize, vals + valoff, (node->btm_nkeys - bound) * base->btm_valsize);
    }

    memcpy(keys + keyoff, key, base->btm_keysize);
    memcpy(vals + valoff, value, base->btm_valsize);
    ++node->btm_nkeys;

    struct scc_btmnode_base **links = scc_btmnode_links(base, node);
    if(bound < node->btm_nkeys - 1u) {
        memmove(links + bound + 2u, links + bound + 1u, (node->btm_nkeys - bound - 1u) * sizeof(*links));
    }
    links[bound + 1u] = child;
}

//? .. c:function:: int scc_btmnode_find_linkindex(\
//?     struct scc_btmap_base const *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict p)
//?
//?     Find and reutrn the index of the given node in the link
//?     array of p
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: Base address of the node to be searched for
//?     :param p: Base address of the parent node whose link array is to be searched
//?     :returns: Index of :code:`node` in the link array of :code:`p`, or :code:`base->btm_order`
//?               if the node is not found in the array
static inline size_t scc_btmnode_find_linkindex(
    struct scc_btmap_base const *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict p
) {
    void *val = scc_btmnode_keys(base, node);
    size_t bound = scc_btmnode_lower_bound(base, p, val) & BOUND_MASK;
    struct scc_btmnode_base **plinks = scc_btmnode_links(base, p);
    for(unsigned i = bound; i < p->btm_nkeys + 1u; ++i) {
        if(plinks[i] == node) {
            return i;
        }
    }

    return base->btm_order;
}

//? .. c:function:: struct scc_btmnode_base *scc_btmnode_split_preemptive(\
//?     struct scc_btmap_base *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *p)
//?
//?     Split the given node in two, moving keys, values and links
//?     as required. The given node is kept as the left child of p
//?     to avoid unnecessary writes
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: Base address of the node to split
//?     :param p: Pointer to node's parent. Should node be the
//?               root, this pointer is NULL and a new root is
//?               allocated from the arena
//?     :returns: Address of the new node allocated for the split, or
//?               :code:`NULL` on allocation failure
static struct scc_btmnode_base *scc_btmnode_split_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *p
) {
    struct scc_btmnode_base *right = scc_arena_alloc(&base->btm_arena);
    if(!right) {
        return 0;
    }

    if(!p) {
        p = scc_arena_alloc(&base->btm_arena);
        if(!p) {
            scc_arena_free(&base->btm_arena, right);
            return 0;
        }
        base->btm_root = p;
        scc_btmap_new_root(base, p, node, right);
    }

    unsigned char *lkeys = scc_btmnode_keys(base, node);
    unsigned char *rkeys = scc_btmnode_keys(base, right);

    unsigned char *lvals = scc_btmnode_vals(base, node);
    unsigned char *rvals = scc_btmnode_vals(base, right);

    struct scc_btmnode_base **llinks = scc_btmnode_links(base, node);
    struct scc_btmnode_base **rlinks = scc_btmnode_links(base, right);

    node->btm_nkeys >>= 1u;
    right->btm_nkeys = node->btm_nkeys;

    right->btm_flags = node->btm_flags;

    memcpy(rkeys, lkeys + (node->btm_nkeys + 1u) * base->btm_keysize, right->btm_nkeys * base->btm_keysize);
    memcpy(rvals, lvals + (node->btm_nkeys + 1u) * base->btm_valsize, right->btm_nkeys * base->btm_valsize);
    if(!scc_btmnode_is_leaf(node)) {
        memcpy(rlinks, llinks + node->btm_nkeys + 1, (node->btm_nkeys + 1u) * sizeof(*rlinks));
    }

    size_t bound = scc_btmnode_find_linkindex(base, node, p);
    assert(bound < base->btm_order);

    unsigned char *pkeys = scc_btmnode_keys(base, p);
    unsigned char *pvals = scc_btmnode_vals(base, p);
    struct scc_btmnode_base **plinks = scc_btmnode_links(base, p);
    if(bound < p->btm_nkeys) {
        memmove(pkeys + (bound + 1u) * base->btm_keysize, pkeys + bound * base->btm_keysize, (p->btm_nkeys - bound) * base->btm_keysize);
        memmove(pvals + (bound + 1u) * base->btm_valsize, pvals + bound * base->btm_valsize, (p->btm_nkeys - bound) * base->btm_valsize);
        memmove(plinks + bound + 1u, plinks + bound, (p->btm_nkeys - bound + 1u) * sizeof(*plinks));
    }
    memcpy(pkeys + bound * base->btm_keysize, lkeys + node->btm_nkeys * base->btm_keysize, base->btm_keysize);
    memcpy(pvals + bound * base->btm_valsize, lvals + node->btm_nkeys * base->btm_valsize, base->btm_valsize);
    plinks[bound + 1u] = right;
    ++p->btm_nkeys;

    return right;
}

//?`.. c:function:: struct scc_btmnode_base *scc_btmnode_split_non_preemptive(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict child, struct scc_btmnode_base *p,\
//?     void *restrict key, void *restrict value)
//?
//?     Split the given node in two, moving keys and values as required. The supplied
//?     key and value are treated as if inserted at their appropriate positions before
//?     the split. The middlemost key-value pair to be moved to the parent node is
//?     written to the first vacant slot int he newly allocated right nodeA
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param node: The node to split
//?     :param child: Child link to insert
//?     :param p: Parent of the given node, or :code:`NULL` if :c:texpr:`node == base->btm_root`
//?     :param key: The key that were to be written to the node, causing the split
//?     :param value: The value corresponding to the key
//?     :returns: Address of the new node allocated for the split, or :code:`NULL`
//?               on allocation failure
static struct scc_btmnode_base *scc_btmnode_split_non_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict child,
    struct scc_btmnode_base *p,
    void *restrict key,
    void *restrict value
) {
    struct scc_btmnode_base *right = scc_arena_alloc(&base->btm_arena);
    if(!right) {
        return 0;
    }
    if(!p) {
        p = scc_arena_alloc(&base->btm_arena);
        if(!p) {
            scc_arena_free(&base->btm_arena, right);
            return 0;
        }
        base->btm_root = p;
        scc_btmap_new_root(base, p, node, right);
    }

    size_t bound = scc_btmnode_lower_bound(base, node, key);
    /* Tree has already been traversed once without
     * finding any exact matches. Eq bit cannot be set */
    assert(bound <= node->btm_nkeys);

    node->btm_nkeys >>= 1u;
    right->btm_nkeys = node->btm_nkeys;
    right->btm_flags = node->btm_flags;

    unsigned char *rkeys = scc_btmnode_keys(base, right);
    unsigned char *rvals = scc_btmnode_vals(base, right);

    unsigned char *lkeys = scc_btmnode_keys(base, node);
    unsigned char *lvals = scc_btmnode_vals(base, node);

    struct scc_btmnode_base **rlinks = scc_btmnode_links(base, right);
    struct scc_btmnode_base **llinks = scc_btmnode_links(base, node);

    void *nkey = scc_btmnode_key(base, right, right->btm_nkeys);
    void *nval = scc_btmnode_value(base, right, right->btm_nkeys);

    if(bound == node->btm_nkeys) {
        memcpy(nkey, key, base->btm_keysize);
        memcpy(nval, value, base->btm_valsize);
        memcpy(rkeys, lkeys + node->btm_nkeys * base->btm_keysize, right->btm_nkeys * base->btm_keysize);
        memcpy(rvals, lvals + node->btm_nkeys * base->btm_valsize, right->btm_nkeys * base->btm_valsize);
        if(!scc_btmnode_is_leaf(node)) {
            *rlinks = child;
            memcpy(rlinks + 1u, llinks + node->btm_nkeys + 1u, right->btm_nkeys * sizeof(*rlinks));
        }
    }
    else if(bound < node->btm_nkeys) {
        memcpy(nkey, lkeys + (node->btm_nkeys - 1u) * base->btm_keysize, base->btm_keysize);
        memcpy(nval, lvals + (node->btm_nkeys - 1u) * base->btm_valsize, base->btm_valsize);
        size_t nmov = node->btm_nkeys - bound - 1u;
        unsigned char *fkey = scc_btmnode_key(base, node, bound);
        unsigned char *fval = scc_btmnode_value(base, node, bound);
        if(nmov) {
            memmove(fkey + base->btm_keysize, fkey, nmov * base->btm_keysize);
            memmove(fval + base->btm_valsize, fval, nmov * base->btm_valsize);
        }
        memcpy(fkey, key, base->btm_keysize);
        memcpy(fval, value, base->btm_valsize);
        memcpy(rkeys, lkeys + node->btm_nkeys * base->btm_keysize, right->btm_nkeys * base->btm_keysize);
        memcpy(rvals, lvals + node->btm_nkeys * base->btm_valsize, right->btm_nkeys * base->btm_valsize);

        if(!scc_btmnode_is_leaf(node)) {
            memcpy(rlinks, llinks + node->btm_nkeys, (right->btm_nkeys + 1u) * sizeof(*rlinks));
            if(nmov) {
                memmove(llinks + bound + 2u, llinks + bound + 1u, (node->btm_nkeys - bound) * sizeof(*llinks));
            }
            llinks[bound + 1u] = child;
        }
    }
    else {
        unsigned char *skey = scc_btmnode_key(base, node, node->btm_nkeys);
        unsigned char *sval = scc_btmnode_value(base, node, node->btm_nkeys);
        memcpy(nkey, skey, base->btm_keysize);
        memcpy(nval, sval, base->btm_valsize);
        skey += base->btm_keysize;
        sval += base->btm_valsize;

        size_t nbef = bound - node->btm_nkeys - 1u;
        if(nbef) {
            size_t nkbytes = nbef * base->btm_keysize;
            size_t nvbytes = nbef * base->btm_valsize;
            memcpy(rkeys, skey, nkbytes);
            memcpy(rvals, sval, nvbytes);

            skey += nkbytes;
            rkeys += nkbytes;

            sval += nvbytes;
            rvals += nvbytes;
        }

        memcpy(rkeys, key, base->btm_keysize);
        memcpy(rvals, value, base->btm_valsize);

        size_t naft = (node->btm_nkeys << 1u) - bound;
        if(naft) {
            memcpy(rkeys + base->btm_keysize, skey, naft * base->btm_keysize);
            memcpy(rvals + base->btm_valsize, sval, naft * base->btm_valsize);
        }

        if(!scc_btmnode_is_leaf(node)) {
            ++nbef;
            memcpy(rlinks, llinks + node->btm_nkeys + 1u, nbef * sizeof(*rlinks));
            rlinks[nbef++] = child;
            if(naft) {
                memcpy(rlinks + nbef, llinks + node->btm_nkeys + nbef, naft * sizeof(*rlinks));
            }
        }
    }

    return right;
}

//? .. c:function:: _Bool scc_btmap_insert_preemptive(struct scc_btmap_base *base, \
//?      void *btmapaddr)
//?
//?     Insert the key-value pair in :c:texpr:`*(void **)btmapaddr``using
//?     :ref:`preemptive splitting <preemptive_splitting>`. The order of the underlying
//?     is assumed to be even.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param btmapaddr: Address of the map handle
static _Bool scc_btmap_insert_preemptive(struct scc_btmap_base *base, void *btmapaddr) {
    struct scc_btmnode_base *curr = base->btm_root;
    struct scc_btmnode_base *p = 0;

    struct scc_btmnode_base *right;
    size_t bound;
    while(1) {
        bound = scc_btmnode_lower_bound(base, curr, *(void **)btmapaddr);
        if(scc_btmnode_keyeq(bound)) {
            scc_btmnode_replace_value(base, curr, *(void **)btmapaddr, bound & BOUND_MASK);
            return true;
        }
        bound &= BOUND_MASK;

        if(curr->btm_nkeys == base->btm_order - 1u) {
            right = scc_btmnode_split_preemptive(base, curr, p);
            if(!right) {
                return false;
            }

            if(bound > curr->btm_nkeys) {
                curr = right;
            }
        }
        if(scc_btmnode_is_leaf(curr)) {
            break;
        }

        bound = scc_btmnode_lower_bound(base, curr, *(void **)btmapaddr);
        p = curr;
        curr = scc_btmnode_child(base, curr, bound);
    }

    scc_btmnode_emplace_leaf(base, curr, *(void **)btmapaddr);
    return true;
}

//? .. c:fuinction:: _Bool scc_btmap_insert_non_preemptive(\
//?     struct scc_btmap_base *base, void *btmapaddr)
//?
//?     Insert the key-value pair in :c:texpr:`(void **)btmapaddr` using :ref:`non-preemptive splitting <non_preemptive_split>`.
//?     Calle donly for trees whose order is odd
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the btmap
//?     :param btmapaddr: Address of the btmap handle
//?     :returns: :code:`true` if the value was successfully inserted, otherwise :code:`false`.
static _Bool scc_btmap_insert_non_preemptive(struct scc_btmap_base *base, void *btmapaddr) {
    _Bool inserted = false;

    scc_stack(struct scc_btmnode_base *) stack = scc_stack_new(struct scc_btmnode_base *);

    /* Root has parent NULL */
    scc_stack_push(&stack, 0);

    /* Splitting root requires new root */
    size_t req_allocs = base->btm_root->btm_nkeys == base->btm_order - 1u;

    size_t bound;
    struct scc_btmnode_base *curr = base->btm_root;
    while(1) {
        if(curr->btm_nkeys == base->btm_order - 1u) {
            ++req_allocs;
        }
        else {
            req_allocs = 0u;
        }

        if(scc_btmnode_is_leaf(curr)) {
            break;
        }

        if(!scc_stack_push(&stack, curr)) {
            goto epilogue;
        }

        bound = scc_btmnode_lower_bound(base, curr, *(void **)btmapaddr);
        if(scc_btmnode_keyeq(bound)) {
            scc_btmnode_replace_value(base, curr, *(void **)btmapaddr, bound & BOUND_MASK);
            inserted = true;
            goto epilogue;
        }
        bound &= BOUND_MASK;
        curr = scc_btmnode_child(base, curr, bound);
    }

    if(curr->btm_nkeys < base->btm_order - 1u) {
        scc_btmnode_emplace_leaf(base, curr, *(void **)btmapaddr);
        inserted = true;
        goto epilogue;
    }

    assert(req_allocs);
    /* Make sure map can't end up in invalid state due to
     * allocation failures */
    if(!scc_arena_reserve(&base->btm_arena, req_allocs)) {
        goto epilogue;
    }

    struct scc_btmnode_base *p;
    struct scc_btmnode_base *right = 0;
    void *key = *(void **)btmapaddr;
    void *value = (unsigned char *)key + base->btm_kvoff;
    while(1) {
        p = scc_stack_top(stack);

        right = scc_btmnode_split_non_preemptive(base, curr, right, p, key, value);
        /* Cannot fail thanks to reserve */
        assert(right);
        key = scc_btmnode_key(base, right, right->btm_nkeys);
        value = scc_btmnode_value(base, right, right->btm_nkeys);

        curr = p;
        if(!curr || curr->btm_nkeys < base->btm_order - 1u) {
            break;
        }

        scc_stack_pop(stack);
    }

    if(!curr) {
        curr = base->btm_root;
    }

    scc_btmnode_emplace(base, curr, right, key, value);
    ++base->btm_size;
    inserted = true;

epilogue:
    scc_stack_free(stack);
    return inserted;
}

//? .. c:function:: void scc_btmnode_rotate_right(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict sibling, struct scc_btmnode_base *restrict p, \
//?     size_t bound)
//?
//?     Rotate key-value pair from left sibling through parent and into the given node.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node: Base address of node to be rotated intgo
//?     :param sibling: Base address of theleft sibling of :code:`node`
//?     :param p: Base address of the parent node of :code:`node` and :code:`sibling`
//?     :param bound: The index of :code:`node` of :code:`p`'s link array
static void scc_btmnode_rotate_right(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict sibling,
    struct scc_btmnode_base *restrict p,
    size_t bound
) {
    unsigned char *nkeys = scc_btmnode_keys(base, node);
    unsigned char *nvals = scc_btmnode_vals(base, node);

    unsigned char *pkeyslot = scc_btmnode_key(base, p, bound - 1u);
    unsigned char *pvalslot = scc_btmnode_value(base, p, bound - 1u);

    unsigned char *skeyslot = scc_btmnode_key(base, sibling, sibling->btm_nkeys - 1u);
    unsigned char *svalslot = scc_btmnode_value(base, sibling, sibling->btm_nkeys - 1u);

    memmove(nkeys + base->btm_keysize, nkeys, node->btm_nkeys * base->btm_keysize);
    memmove(nvals + base->btm_valsize, nvals, node->btm_nkeys * base->btm_valsize);

    memcpy(nkeys, pkeyslot, base->btm_keysize);
    memcpy(nvals, pvalslot, base->btm_valsize);

    memcpy(pkeyslot, skeyslot, base->btm_keysize);
    memcpy(pvalslot, svalslot, base->btm_valsize);

    struct scc_btmnode_base **nlinks = scc_btmnode_links(base, node);
    struct scc_btmnode_base *subtree = scc_btmnode_child(base, sibling, sibling->btm_nkeys--);

    memmove(nlinks + 1u, nlinks, ++node->btm_nkeys * sizeof(*nlinks));
    nlinks[0] = subtree;
}

//? .. c:function:: void scc_btmnode_rotate_left(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict sibling, struct scc_btmnode_base *restrict p, \
//?     size_t bound)
//?
//?     Rotate value from right sibling through parent and into the given node.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node: Base address of the node to rotated into
//?     :param sibling: Base address of the right sibling of :code:`node`
//?     :param p: Base address of the parent node of :code:`node` and :code:`sibling`
//?     :param bound: The index of :code:`node` in :code:`p`'s link array
static void scc_btmnode_rotate_left(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict sibling,
    struct scc_btmnode_base *restrict p,
    size_t bound
) {
    unsigned char *nkeyslot = scc_btmnode_key(base, node, node->btm_nkeys);
    unsigned char *nvalslot = scc_btmnode_value(base, node, node->btm_nkeys);

    unsigned char *pkeyslot = scc_btmnode_key(base, p, bound);
    unsigned char *pvalslot = scc_btmnode_value(base, p, bound);

    unsigned char *skeys = scc_btmnode_keys(base, sibling);
    unsigned char *svals = scc_btmnode_vals(base, sibling);

    memcpy(nkeyslot, pkeyslot, base->btm_keysize);
    memcpy(nvalslot, pvalslot, base->btm_valsize);

    memcpy(pkeyslot, skeys, base->btm_keysize);
    memcpy(pvalslot, svals, base->btm_valsize);

    struct scc_btmnode_base **nlinks = scc_btmnode_links(base, node);
    struct scc_btmnode_base **slinks = scc_btmnode_links(base, sibling);

    nlinks[++node->btm_nkeys] = slinks[0];
    memmove(slinks, slinks + 1u, sibling->btm_nkeys-- * sizeof(*slinks));

    memmove(skeys, skeys + base->btm_keysize, sibling->btm_nkeys * base->btm_keysize);
    memmove(svals, svals + base->btm_valsize, sibling->btm_nkeys * base->btm_valsize);
}

//? .. c:function:: void scc_btmnode_overwrite(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict leaf, \
//?     struct scc_btmnode_base *restrict found, size_t fbound, _Bool predecessor)
//?
//?    Swap the in-order predecessor or successor with the element at the index
//?    :code:`fbound`  in the found node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param leaf: Leaf node to copy the predecessor or successor from
//?     :param found: Internal node whose key-value pair is to be overwritten
//?     :param fbound: Lower bound of the key-value pair to be overwritten in the
//?                    internal node
//?     :param predecessor: :code:`true` if the key-value pair is to be overwritten
//?                         with the in-order predecessor. If :code:`false`, the key-value
//?                         pair is overwritten with the in-order successor
static inline void scc_btmnode_overwrite(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict leaf,
    struct scc_btmnode_base *restrict found,
    size_t fbound,
    _Bool predecessor
) {
    size_t const idx = predecessor ? leaf->btm_nkeys - 1u : 0u;
    unsigned char *key = scc_btmnode_key(base, leaf, idx);
    unsigned char *value = scc_btmnode_value(base, leaf, idx);
    memcpy(scc_btmnode_key(base, found, fbound), key, base->btm_keysize);
    memcpy(scc_btmnode_value(base, found, fbound), value, base->btm_valsize);
    --leaf->btm_nkeys;
    if(!predecessor) {
        memmove(key, key + base->btm_keysize, leaf->btm_nkeys * base->btm_keysize);
        memmove(value, value + base->btm_valsize, leaf->btm_nkeys * base->btm_valsize);
    }
}

//? .. c:function:: void scc_btmnode_merge(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict sibling, struct scc_btmnode_base *restrict p, \
//?     size_t bound)
//?
//?     Generic merging of the given node with its left sibling, leaving parent links
//?     untouched
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Baser address of the ``btmap``
//?     :param node: Base address of the node to be rotated into
//?     :param sibling: Base address of the left sibling of :code:`node`
//?     :param p: Base address of the parent node of :code:`node` and :code:`sibling`
//?     :param bound: The index of :code:`node` in :code:`p`'s link array
//?     :returns: Number of links that were moved left in :code:`p`'s link array
static size_t scc_btmnode_merge(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict sibling,
    struct scc_btmnode_base *restrict p,
    size_t bound
) {
    unsigned char *nkeys = scc_btmnode_keys(base, node);
    unsigned char *nvals = scc_btmnode_vals(base, node);

    unsigned char *skeyslot = scc_btmnode_key(base, sibling, sibling->btm_nkeys);
    unsigned char *svalslot = scc_btmnode_value(base, sibling, sibling->btm_nkeys);

    unsigned char *pkeyslot = scc_btmnode_key(base, p, bound);
    unsigned char *pvalslot = scc_btmnode_value(base, p, bound);

    /* Append key-value pair from parent to left sibling */
    memcpy(skeyslot, pkeyslot, base->btm_keysize);
    memcpy(svalslot, pvalslot, base->btm_valsize);

    /* Keys and values from node to left sibling */
    memcpy(skeyslot + base->btm_keysize, nkeys, node->btm_nkeys * base->btm_keysize);
    memcpy(svalslot + base->btm_valsize, nvals, node->btm_nkeys * base->btm_valsize);

    struct scc_btmnode_base **nlinks = scc_btmnode_links(base, node);
    struct scc_btmnode_base **slinks = scc_btmnode_links(base, sibling);

    if(!scc_btmnode_is_leaf(node)) {
        assert(!scc_btmnode_is_leaf(sibling));
        memcpy(slinks + sibling->btm_nkeys + 1u, nlinks, (node->btm_nkeys + 1u) * sizeof(*slinks));
    }

    sibling->btm_nkeys += node->btm_nkeys + 1u;
    assert(sibling->btm_nkeys < base->btm_order);

    size_t nmov = p->btm_nkeys - bound - 1u;
    if(nmov) {
        memmove(pkeyslot, pkeyslot + base->btm_keysize, nmov * base->btm_keysize);
        memmove(pvalslot, pvalslot + base->btm_valsize, nmov * base->btm_valsize);
    }
    if(!--p->btm_nkeys) {
        assert(!scc_bits_is_even(base->btm_order) || p == base->btm_root);
        if(scc_bits_is_even(base->btm_order)) {
            base->btm_root = sibling;
        }
    }

    return nmov;
}

//? .. c:function:: void scc_btmnode_merge_left_non_preemptive(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict sibling, struct scc_btmnode_base *restrict p, \
//?     size_t bound)
//?
//?     Merge the given node with its left sibling
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node. Base address of node to be rotated into
//?     :param sibling: Base address ofhte left sibling of :code:`node`
//?     :param p: Base address of the parent node of :code:`node` and :code:`sibling`
//?     :param bound: The index of :code:`node` in :code:`p`'s link array
static inline void scc_btmnode_merge_left_non_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict sibling,
    struct scc_btmnode_base *restrict p,
    size_t bound
) {
    size_t nmov = scc_btmnode_merge(base, node, sibling, p, bound - 1u) + 1u;
    if(bound <= p->btm_nkeys) {
        struct scc_btmnode_base **plinks = scc_btmnode_links(base, p);
        memmove(plinks + bound, plinks + bound + 1u, nmov * sizeof(*plinks));
    }
    scc_arena_try_free(&base->btm_arena, node);
}

//? .. c:function:: void scc_btmnode_merge_left_preemptive(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict sibling, struct scc_btmnode_base *restrict p, \
//?     size_t bound)
//?
//?     Merge the given node with its left sibling, freeing the parent node
//?     if required
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node: Base address of the node to be rotated intoA
//?     :param sibling: Base address ofhte left sibling of :code:`node`
//?     :param p: Base address of the parent node of :code:`node` and :code:`sibling`
//?     :param bound: The index of :code:`node` in :code:`p`'s link array
static inline void scc_btmnode_merge_left_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict sibling,
    struct scc_btmnode_base *restrict p,
    size_t bound
) {
    scc_btmnode_merge_left_non_preemptive(base, node, sibling, p, bound);
    if(!p->btm_nkeys) {
        scc_arena_try_free(&base->btm_arena, p);
    }
}

//? .. c:function:: void scc_btmnode_base_right_non_preemptive(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict sibling, struct scc_btmnode_base *restrict p, \
//?     size_t bound)
//?
//?     Merge the given node with its right sibling
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node: Base address of the node to be rotated into
//?     :param sibling: Base address of the right sibling of :code:`node`
//?     :param p: Base address of the parent node of :code:`node` and :code:`sibling`
//?     :param bound: The index of :code:`node` in :code:`p`'s link array
static inline void scc_btmnode_merge_right_non_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict sibling,
    struct scc_btmnode_base *restrict p,
    size_t bound
) {
    size_t nmov = scc_btmnode_merge(base, sibling, node, p, bound);
    if(p->btm_nkeys) {
        struct scc_btmnode_base **plinks = scc_btmnode_links(base, p);
        memmove(plinks + bound + 1u, plinks + bound + 2u, nmov * sizeof(*plinks));
    }
    scc_arena_try_free(&base->btm_arena, sibling);
}

//? .. c:function:: void scc_btmnode_merge_right_preemptive(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict sibling, struct scc_btmnode_base *restrict p, \
//?     size_t bound)
//?
//?     Merge the given node with its right sibling, freeing the parent node as required
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node: Base address of the node to be rotated into
//?     :param sibling: Base address of the right sibling of :code:`node`
//?     :param p: Base address of the parent node of :code:`node` and :code:`sibling`
//?     :param bound: The index of :code:`node` in :code:`p`'s link array
static inline void scc_btmnode_merge_right_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict sibling,
    struct scc_btmnode_base *restrict p,
    size_t bound
) {
    scc_btmnode_merge_right_non_preemptive(base, node, sibling, p, bound);
    if(!p->btm_nkeys) {
        scc_arena_try_free(&base->btm_arena, p);
    }
}

//? .. c:function:: void scc_btmap_balance_preemptive(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict next, \
//?     struct scc_btmnode_base *restrict curr, size_t bound)
//?
//?     Balance ``btmap`` for preemptive removal
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param next: The next node to traverse
//?     :param curr: The current node being traversed. Parent of :code:`next`
//?     :param bound: Index of :code:`next` in the link array of :code:`curr`
static struct scc_btmnode_base *scc_btmap_balance_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict next,
    struct scc_btmnode_base *restrict curr,
    size_t bound
) {
    size_t const borrow_lim = base->btm_order >> 1u;

    struct scc_btmnode_base *sibling = 0;
    if(bound) {
        sibling = scc_btmnode_child(base, curr, bound - 1u);
        if(sibling->btm_nkeys >= borrow_lim) {
            scc_btmnode_rotate_right(base, next, sibling, curr, bound);
            return next;
        }
    }

    if(bound < curr->btm_nkeys) {
        sibling = scc_btmnode_child(base, curr, bound + 1u);
        if(sibling->btm_nkeys >= borrow_lim) {
            scc_btmnode_rotate_left(base, next, sibling, curr, bound);
        }
        else {
            scc_btmnode_merge_right_preemptive(base, next, sibling, curr, bound);
        }
        return next;
    }

    assert(sibling);
    scc_btmnode_merge_left_preemptive(base, next, sibling, curr, bound);
    return sibling;
}

//? .. c:function:: void scc_btmnode_remove_leaf(\
//?     struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, \
//?     size_t bound)
//?
//?     Remove the key-value pair at the specified index from the given leaf node. The node must
//?     contain at least 2 keys-value pairs
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param node: Leaf node from which the key-value pair is to be removed
//?     :param n: Index of the value to be removed
static inline void scc_btmnode_remove_leaf(struct scc_btmap_base *restrict base, struct scc_btmnode_base *restrict node, size_t n) {
    assert(!scc_bits_is_even(base->btm_order) || node->btm_nkeys > 1u || node == base->btm_root);

    size_t nmov = node->btm_nkeys - n - 1u;
    --node->btm_nkeys;
    if(!nmov) {
        return;
    }

    unsigned char *key = scc_btmnode_key(base, node, n);
    unsigned char *val = scc_btmnode_value(base, node, n);

    memmove(key, key + base->btm_keysize, nmov * base->btm_keysize);
    memmove(val, val + base->btm_valsize, nmov * base->btm_valsize);
}

//? .. c:function:: _Bool scc_btmap_remove_preemptive(\
//?     struct scc_btmap_base *restrict base, void *restrict btmap)
//?
//?     Find and remove the value stored in the :code:`btm_curr` field using preemptive
//?     merging
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param btmap: ``btmap`` handle
//?     :returns: :code:`true` if the key-value pair vwas removed, :code:`false` if the
//?               key was not found
static _Bool scc_btmap_remove_preemptive(struct scc_btmap_base *restrict base, void *restrict btmap) {
    size_t const borrow_lim = base->btm_order >> 1u;
    _Bool swap_pred = true;

    size_t fbound;
    struct scc_btmnode_base *found = 0;

    struct scc_btmnode_base *curr = base->btm_root;

    size_t bound;
    unsigned char *key;
    struct scc_btmnode_base *next;

    while(1) {
        if(!found) {
            bound = scc_btmnode_lower_bound(base, curr, btmap);
        }
        else if(found == curr) {
            bound = fbound;
        }
        else {
            bound = swap_pred * curr->btm_nkeys;
        }

        next = scc_btmnode_child(base, curr, bound & BOUND_MASK);
        key = scc_btmnode_key(base, curr, bound & BOUND_MASK);

        if(!found && scc_btmnode_keyeq(bound)) {
            bound &= BOUND_MASK;
            fbound = bound;
            found = curr;
            if(scc_btmnode_is_leaf(curr)) {
                break;
            }

            if(next->btm_nkeys < borrow_lim && bound < curr->btm_nkeys) {
                struct scc_btmnode_base *right = scc_btmnode_child(base, curr, bound + 1u);
                if(right->btm_nkeys >= borrow_lim) {
                    next = right;
                    swap_pred = false;
                }
                else {
                    scc_btmnode_merge_right_preemptive(base, next, right, curr, bound);
                    found = next;
                    fbound = scc_btmnode_lower_bound(base, found, btmap);
                    assert(!scc_btmnode_keyeq(fbound));
                }
            }
        }
        else if(scc_btmnode_is_leaf(curr)) {
            break;
        }
        else if(next->btm_nkeys < borrow_lim) {
            next = scc_btmap_balance_preemptive(base, next, curr, bound);
            /* Found key may be rotated into next node when balancing. If it is,
             * the key is always in the next node */
            if(curr == found) {
                key = scc_btmnode_key(base, found, fbound);
                if(found->btm_nkeys <= fbound || base->btm_compare(key, btmap)) {
                    found = next;
                    fbound = scc_btmnode_lower_bound(base, found, btmap);
                    assert(!scc_btmnode_keyeq(fbound));
                }
            }
        }

        curr = next;
    }

    if(!found) {
        return false;
    }

    if(scc_btmnode_is_leaf(found)) {
        scc_btmnode_remove_leaf(base, found, fbound);
    }
    else {
        scc_btmnode_overwrite(base, curr, found, fbound, swap_pred);
    }

    --base->btm_size;
    return true;
}

//? .. c:function:: void scc_btmap_balance_non_preemptive(\
//?     struct scc_btmap_base *rewstrict base, struct scc_btmnode_base *restrict curr, \
//?     struct scc_btmnode_base **restrict nodes, size_t bounds)
//?
//?     Traverse the tree back towards the root, balancing as needed
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param curr: Leaf node in which to start the traversal
//?     :param nodes: Stack of nodes traversed while finding the leaf
//?     :param bounds: Stack of bounds computed while finding ht eleaf. The
//?                    nth bound in the stack is the index of the link in the
//?                    nth node to obtain the n+1th node
static void scc_btmap_balance_non_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict curr,
    scc_stack(struct scc_btmnode_base *) nodes,
    scc_stack(size_t) bounds
) {
    assert(scc_stack_size(nodes) == scc_stack_size(bounds));

    size_t const borrow_lim = (base->btm_order >> 1u) + 1u;
    struct scc_btmnode_base *sibling = 0;
    struct scc_btmnode_base *child;
    size_t bound;

    while(1) {
        child = curr;
        curr = scc_stack_top(nodes);
        bound = scc_stack_top(bounds);

        if(!curr || child->btm_nkeys >= borrow_lim - 1u) {
            break;
        }

        scc_stack_pop(nodes);
        scc_stack_pop(bounds);

        if(bound) {
            sibling = scc_btmnode_child(base, curr, bound - 1u);
            if(sibling->btm_nkeys >= borrow_lim) {
                scc_btmnode_rotate_right(base, child, sibling ,curr, bound);
                continue;
            }
        }

        if(bound < curr->btm_nkeys) {
            sibling = scc_btmnode_child(base, curr, bound + 1u);
            if(sibling->btm_nkeys >= borrow_lim) {
                scc_btmnode_rotate_left(base, child, sibling, curr, bound);
                continue;
            }
            scc_btmnode_merge_right_non_preemptive(base, child, sibling, curr, bound);
            continue;
        }

        assert(sibling);
        scc_btmnode_merge_left_non_preemptive(base, child, sibling, curr, bound);
    }

    if(!base->btm_root->btm_nkeys) {
        child = *scc_btmnode_links(base, base->btm_root);
        scc_arena_try_free(&base->btm_arena, base->btm_root);
        base->btm_root = child;
    }
}

//? .. c:function:: _Bool scc_btmap_remove_non_preemptive(\
//?     struct scc_btmap_base *restrict base, void *restrict btmap)
//?
//?     Find and remove the key-value pair identified by the key in the :ref:`btm_curr <kvpair_btm_curr>`
//?     field using non-preemptive merging
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the ``btmap``
//?     :param btmap: ``btmap`` handle
//?     :returns: :code:`true` if the key-value pair was removed, :code:`false` if the
//?               key was not found
static _Bool scc_btmap_remove_non_preemptive(struct scc_btmap_base *restrict base, void *restrict btmap) {
    size_t const borrow_lim = (base->btm_order >> 1u) + 1u;
    size_t const origsz = base->btm_size;

    _Bool swap_pred = true;

    scc_stack(struct scc_btmnode_base *) nodes = scc_stack_new(struct scc_btmnode_base *);
    scc_stack(size_t) bounds = scc_stack_new(size_t);

    struct scc_btmnode_base *found = 0;
    size_t fbound;

    size_t bound;

    struct scc_btmnode_base *curr = base->btm_root;
    struct scc_btmnode_base *next;

    /* Root has parent NULL */
    if(!scc_stack_push(&nodes, 0) || !scc_stack_push(&bounds, 0)) {
        goto epilogue;
    }

    _Bool keyeq = false;

    while(1) {
        if(!found) {
            bound = scc_btmnode_lower_bound(base, curr, btmap);
        }
        else {
            bound = swap_pred * curr->btm_nkeys;
        }

        next = scc_btmnode_child(base, curr, bound);
        keyeq = scc_btmnode_keyeq(bound);
        bound &= BOUND_MASK;

        if(keyeq) {
            assert(!found);
            found = curr;
            fbound = bound;

            if(scc_btmnode_is_leaf(curr)) {
                break;
            }

            if(next->btm_nkeys < borrow_lim && bound < curr->btm_nkeys) {
                struct scc_btmnode_base *right = scc_btmnode_child(base, curr, bound + 1u);
                if(right->btm_nkeys >= borrow_lim) {
                    ++bound;
                    next = right;
                    swap_pred = false;
                }
            }
        }

        if(scc_btmnode_is_leaf(curr)) {
            break;
        }

        if(scc_btmnode_is_leaf(curr)) {
            break;
        }

        if(!scc_stack_push(&nodes, curr) || !scc_stack_push(&bounds, bound)) {
            goto epilogue;
        }
        curr = next;
    }

    if(!found) {
        goto epilogue;
    }

    if(scc_btmnode_is_leaf(found)) {
        scc_btmnode_remove_leaf(base, found, fbound);
    }
    --base->btm_size;

    if(base->btm_size) {
        scc_btmap_balance_non_preemptive(base, curr, nodes, bounds);
    }

epilogue:
    scc_stack_free(nodes);
    scc_stack_free(bounds);

    return base->btm_size < origsz;
}

void *scc_btmap_impl_new(void *base, size_t coff, size_t rootoff) {
#define base ((struct scc_btmap_base *)base)
    size_t fwoff = coff - offsetof(struct scc_btmap_base, btm_fwoff) - sizeof(base->btm_fwoff);
    assert(fwoff <= UCHAR_MAX);
    base->btm_fwoff = (unsigned char)fwoff;
    scc_btmap_root_init(base, (unsigned char *)base + rootoff);
    unsigned char *btmap = (unsigned char *)base + coff;
    scc_btmap_set_bkoff(btmap, fwoff);
    return btmap;
#undef base
}

void scc_btmap_free(void *btmap) {
    struct scc_btmap_base *base = scc_btmap_impl_base(btmap);
    scc_arena_release(&base->btm_arena);
}

_Bool scc_btmap_impl_insert(void *btmapaddr) {
    struct scc_btmap_base *base = scc_btmap_impl_base(*(void **)btmapaddr);
    if(scc_bits_is_even(base->btm_order)) {
        return scc_btmap_insert_preemptive(base, btmapaddr);
    }
    return scc_btmap_insert_non_preemptive(base, btmapaddr);
}

void *scc_btmap_impl_find(void *btmap) {
    struct scc_btmap_base *base = scc_btmap_impl_base(btmap);

    struct scc_btmnode_base *curr = base->btm_root;

    size_t bound;
    while(1) {
        bound = scc_btmnode_lower_bound(base, curr, btmap);
        if(scc_btmnode_keyeq(bound)) {
            return scc_btmnode_value(base, curr, bound & BOUND_MASK);
        }

        if(scc_btmnode_is_leaf(curr)) {
            break;
        }

        curr = scc_btmnode_child(base, curr, bound);
    }

    return 0;
}

_Bool scc_btmap_impl_remove(void *btmap) {
    struct scc_btmap_base *base = scc_btmap_impl_base(btmap);
    if(scc_bits_is_even(base->btm_order)) {
        return scc_btmap_remove_preemptive(base, btmap);
    }
    return scc_btmap_remove_non_preemptive(base, btmap);
}
