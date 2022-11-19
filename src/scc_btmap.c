#include <scc/scc_algorithm.h>
#include <scc/scc_bits.h>
#include <scc/scc_btmap.h>

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

//? .. c:function:: void scc_btnode_flags_clear(struct scc_btmnode_base *node)
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
//?     size_t n, size_t valuesize)
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
//?     :param valuesize: Size of the values in the map
static inline void *scc_btmnode_value(
    struct scc_btmap_base const *restrict base,
    struct scc_btmnode_base *restrict node,
    size_t n,
    size_t valuesize
) {
    assert(n < node->btm_nkeys);
    return ((unsigned char *)scc_btmnode_vals(base, node)) + n * valuesize;
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
//?     void const *restrict value,
//?     size_t keysize)
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
//?     :param keysize: Size of the keys in the node
//?     :returns: The lower bound of :code:`value` in the key array of :code:`node`.
static inline size_t scc_btmnode_lower_bound(
    struct scc_btmap_base const *base,
    struct scc_btmnode_base *node,
    void const *restrict value,
    size_t keysize
) {
    if(!node->btm_nkeys) {
        return 0u;
    }
    return scc_algo_lower_bound_eq(value, scc_btmnode_keys(base, node), node->btm_nkeys, keysize, base->btm_compare);
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
//?     size_t bound, size_t valuesize)
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
//?     :param valuesize: Size of the values in the map
static inline void scc_btmnode_replace_value(
    struct scc_btmap_base const *restrict base,
    struct scc_btmnode_base *restrict node,
    void const *restrict kvpair,
    size_t bound,
    size_t valuesize
) {
    void *val = scc_btmnode_value(base, node, bound, valuesize);
    memcpy(val, (unsigned char const *)kvpair + base->btm_kvoff, valuesize);
}

//? .. c:function:: size_t scc_btmnode_empalce_leaf(\
//?     struct scc_btmap_base *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     void *restrict kvpair, \
//?     size_t elemsize, \
//?     size_t valsize)
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
//?     :param keysize: Size of the keys in the map
//?     :param valsize: Size of the values in the map
//?     :returns: The lower bound of the newly inserted element
static size_t scc_btmnode_emplace_leaf(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    void *restrict kvpair,
    size_t keysize,
    size_t valsize
) {
    size_t bound = scc_btmnode_lower_bound(base, node, kvpair, keysize) & BOUND_MASK;
    unsigned char *keys = scc_btmnode_keys(base, node);
    unsigned char *vals = scc_btmnode_vals(base, node);

    size_t keyoff = bound * keysize;
    size_t valoff = bound * valsize;

    if(bound < node->btm_nkeys) {
        memmove(keys + keyoff + keysize, keys + keyoff, (node->btm_nkeys - bound) * keysize);
        memmove(vals + valoff + valsize, vals + valoff, (node->btm_nkeys - bound) * valsize);
    }

    memcpy(keys + keyoff, kvpair, keysize);
    memcpy(vals + valoff, (unsigned char *)kvpair + base->btm_kvoff, valsize);
    ++node->btm_nkeys;
    return bound;
}

//? .. c:function:: int scc_btmnode_find_linkindex(\
//?     struct scc_btmap_base const *restrict base, \
//?     struct scc_btmnode_base *restrict node, \
//?     struct scc_btmnode_base *restrict p, \
//?     size_t keysize)
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
//?     :param keysize: Size of the keys in the node
//?     :returns: Index of :code:`node` in the link array of :code:`p`, or :code:`base->btm_order`
//?               if the node is not found in the array
static inline size_t scc_btmnode_find_linkindex(
    struct scc_btmap_base const *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *restrict p,
    size_t keysize
) {
    void *val = scc_btmnode_keys(base, node);
    size_t bound = scc_btmnode_lower_bound(base, p, val, keysize) & BOUND_MASK;
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
//?     struct scc_btmnode_base *p, \
//?     size_t keysize, \
//?     size_t elemsize)
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
//?     :param keysize: Size of the keys stored in the map
//?     :param valsize: Size of the values stored in the map
//?     :returns: Address of the new node allocated for the split, or
//?               :code:`NULL` on allocation failure
static struct scc_btmnode_base *scc_btmnode_split_preemptive(
    struct scc_btmap_base *restrict base,
    struct scc_btmnode_base *restrict node,
    struct scc_btmnode_base *p,
    size_t keysize,
    size_t valsize
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

    memcpy(rkeys, lkeys + (node->btm_nkeys + 1u) * keysize, right->btm_nkeys * keysize);
    memcpy(rvals, lvals + (node->btm_nkeys + 1u) * valsize, right->btm_nkeys * valsize);
    if(!scc_btmnode_is_leaf(node)) {
        memcpy(rlinks, llinks + node->btm_nkeys + 1, (node->btm_nkeys + 1u) * sizeof(*rlinks));
    }

    size_t bound = scc_btmnode_find_linkindex(base, node, p, keysize);
    assert(bound < base->btm_order);

    unsigned char *pkeys = scc_btmnode_keys(base, p);
    unsigned char *pvals = scc_btmnode_vals(base, p);
    struct scc_btmnode_base **plinks = scc_btmnode_links(base, p);
    if(bound < p->btm_nkeys) {
        memmove(pkeys + (bound + 1u) * keysize, pkeys + bound * keysize, (p->btm_nkeys - bound) * keysize);
        memmove(pvals + (bound + 1u) * valsize, pvals + bound * valsize, (p->btm_nkeys - bound) * valsize);
        memmove(plinks + bound + 1u, plinks + bound, (p->btm_nkeys - bound + 1u) * sizeof(*plinks));
    }
    memcpy(pkeys + bound * keysize, lkeys + node->btm_nkeys + keysize, keysize);
    memcpy(pvals + bound * valsize, lvals + node->btm_nkeys + valsize, valsize);
    plinks[bound + 1u] = right;
    ++p->btm_nkeys;

    return right;
}

//? .. c:function:: _Bool scc_btmap_insert_preemptive(struct scc_btmap_base *base, \
//?      void *btmapaddr, size_t keysize, size_t valsize)
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
//?     :param keysize: Size of the keys stored in the map
//?     :param valsize: Size of the values stored in the map
static _Bool scc_btmap_insert_preemptive(struct scc_btmap_base *base, void *btmapaddr, size_t keysize, size_t valsize) {
    struct scc_btmnode_base *curr = base->btm_root;
    struct scc_btmnode_base *p = 0;

    struct scc_btmnode_base *right;
    size_t bound;
    while(true) {
        bound = scc_btmnode_lower_bound(base, curr, *(void **)btmapaddr, keysize);
        if(scc_btmnode_keyeq(bound)) {
            scc_btmnode_replace_value(base, curr, *(void **)btmapaddr, bound & BOUND_MASK, valsize);
            return true;
        }
        bound &= BOUND_MASK;

        if(curr->btm_nkeys == base->btm_order - 1u) {
            right = scc_btmnode_split_preemptive(base, curr, p, keysize, valsize);
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

        bound = scc_btmnode_lower_bound(base, curr, *(void **)btmapaddr, keysize);
        p = curr;
        curr = scc_btmnode_child(base, curr, bound);
    }

    scc_btmnode_emplace_leaf(base, curr, *(void **)btmapaddr, keysize, valsize);
    ++base->btm_size;
    return true;
}

void scc_btmap_free(void *btmap) {
    struct scc_btmap_base *base = scc_btmap_impl_base(btmap);
    scc_arena_release(&base->btm_arena);
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

_Bool scc_btmap_impl_insert(void *btmapaddr, size_t keysize, size_t valsize) {
    struct scc_btmap_base *base = scc_btmap_impl_base(*(void **)btmapaddr);
    if(scc_bits_is_even(base->btm_order)) {
        return scc_btmap_insert_preemptive(base, btmapaddr, keysize, valsize);
    }
    return false;
}
