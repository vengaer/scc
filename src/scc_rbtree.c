#include <scc/scc_mem.h>
#include <scc/scc_rbtree.h>

#include <assert.h>
#include <limits.h>

#define rn_left     rn_bare.un_link.node.left
#define rn_right    rn_bare.un_link.node.right
#define rn_parent   rn_bare.un_link.temp.parent
#define rn_color    rn_bare.color
#define rn_flags    rn_bare.flags

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

static void scc_rbnode_set_npad(
    struct scc_rbnode *restrict node,
    struct scc_rbtree const *restrict tree
);
static unsigned scc_rbnode_npad(void const *rbnode);

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


void *scc_rbtree_impl_init(struct scc_rbtree *tree) {
    tree->rb_size = 0u;
    tree->rb_sentinel.un_link.node.left = &tree->rb_sentinel;
    tree->rb_sentinel.un_link.node.right = &tree->rb_sentinel;
    tree->rb_sentinel.flags = SCC_RBLEAF;

    struct scc_rbnode *node = scc_arena_alloc(&tree->rb_arena);
    node->rn_parent = tree;
    scc_rbnode_set_npad(node, tree);
    return (unsigned char *)node + tree->rb_baseoff;
}

void scc_rbtree_free(void *addr) {
    scc_arena_release(&scc_rbtree_from_handle(addr)->rb_arena);
}

size_t scc_rbtree_size(void const *addr) {
    return scc_rbtree_from_handle_qual(addr, const)->rb_size;
}
