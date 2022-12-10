#include <scc/rbmap.h>

#include <stdbool.h>
#include <string.h>

#define rb_root rb_sentinel.rs_left

size_t scc_rbmap_size(void const *map);
_Bool scc_rbmap_empty(void const *map);
void scc_rbmap_free(void *map);
void const *scc_rbmap_impl_iterstop(void const *map);

//? .. c:function:: struct scc_rbnode_base *scc_rbmap_leftmost(\
//?     struct scc_rbnode_base *root)
//?
//?     Find the leftmost node in the subtree spanning from the given root
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param root: The root of the subtree
//?     :returns:    Address of the leftmost node in the subtree
static inline struct scc_rbnode_base *scc_rbmap_leftmost(struct scc_rbnode_base *root) {
    while(!scc_rbnode_thread(root, scc_rbdir_left)) {
        root = root->rn_left;
    }
    return root;
}

//? .. c:function:: struct scc_rbnode_base *scc_rbmap_rightmost(\
//?     struct scc_rbnode_base *root)
//?
//?     Find the rightmost node in the subtree spanning from the given root
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param root: The root of the subtree
//?     :returns:    Address of the rightmost node in the subtree
static inline struct scc_rbnode_base *scc_rbmap_rightmost(struct scc_rbnode_base *root) {
    while(!scc_rbnode_thread(root, scc_rbdir_right)) {
        root = root->rn_right;
    }
    return root;
}

//? .. c:function:: void const *scc_rbmnode_key(\
//?     struct scc_rbtree_base const *restrict base, \
//?     void const *restrict node)
//?
//?     Compute and return const-qualified ponter to the key
//?     stored in the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the rbmap
//?     :param node: Base address of the node whose key is to be found
//?     :returns:    Address of the key stored in the node
static inline void const *scc_rbmnode_key(
    struct scc_rbtree_base const *restrict base,
    void const *restrict node
) {
    return scc_rbnode_value_qual(base, node, const);
}

//? .. c:function:: void *scc_rbmnode_value(\
//?     struct scc_rbtree_base const *restrict base, \
//?     void const *restrict node, size_t valoff)
//?
//?     Compute and return ponter to the value stored in
//?     the given node
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base:   Base address of the rbmap
//?     :param node:   Base address of the node whose value is to be found
//?     :param valoff: Offset of the value in the internal :ref:`pair struct <scc_rbmap_impl_pair>`
static inline void *scc_rbmnode_value(
    struct scc_rbtree_base const *restrict base,
    void *restrict node,
    size_t valoff
) {
    return (unsigned char *)scc_rbnode_value(base, node) + valoff;
}

_Bool scc_rbmap_impl_insert(void *rbmapaddr, size_t elemsize, size_t valoff) {
    unsigned char *addr = scc_rbtree_impl_generic_insert(rbmapaddr, elemsize);
    if(!addr) {
        return false;
    }

    if(addr == *(void **)rbmapaddr) {
        return true;
    }

    /* Preexisting entry, update value */
    unsigned char const *raddr = ((unsigned char const *)*(void **)rbmapaddr) + valoff;
    memcpy(addr + valoff, raddr, elemsize - valoff);
    return true;
}

void *scc_rbmap_impl_find(void *map, size_t valoff) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(map);
    struct scc_rbnode_base *p = (void *)&base->rb_sentinel;
    struct scc_rbnode_base *n = base->rb_root;

    enum scc_rbdir dir = scc_rbdir_left;
    int rel;

    while(!scc_rbnode_thread(p, dir)) {
        rel = base->rb_compare(scc_rbmnode_key(base, n), map);
        if(!rel) {
            return scc_rbmnode_value(base, n, valoff);
        }

        dir = rel <= 0;
        p = n;
        n = scc_rbnode_link_qual(n, dir, const);
    }

    return 0;
}

void *scc_rbmap_impl_leftmost_pair(void *map) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(map);
    struct scc_rbnode_base *leftmost = scc_rbmap_leftmost(base->rb_root);
    return scc_rbnode_value(base, leftmost);
}

void *scc_rbmap_impl_rightmost_pair(void *map) {
    struct scc_rbtree_base *base = scc_rbtree_impl_base(map);
    struct scc_rbnode_base *rightmost = scc_rbmap_rightmost(base->rb_root);
    return scc_rbnode_value(base, rightmost);
}

void *scc_rbmap_impl_successor(void *iter) {
    struct scc_rbnode_base *node = scc_rbnode_impl_base(iter);
    size_t const offset = (unsigned char const *)iter - (unsigned char const *)node;
    if(scc_rbnode_thread(node, scc_rbdir_right)) {
        node = node->rn_right;
    }
    else {
        node = scc_rbmap_leftmost(node->rn_right);
    }

    return (unsigned char *)node + offset;
}

void *scc_rbmap_impl_predecessor(void *iter) {
    struct scc_rbnode_base *node = scc_rbnode_impl_base(iter);
    size_t const offset = (unsigned char const *)iter - (unsigned char const *)node;
    if(scc_rbnode_thread(node, scc_rbdir_left)) {
        node = node->rn_left;
    }
    else {
        node = scc_rbmap_rightmost(node->rn_left);
    }

    return (unsigned char *)node + offset;
}
