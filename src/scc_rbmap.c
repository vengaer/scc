#include <scc/scc_rbmap.h>

#define rb_root rb_sentinel.rs_left

size_t scc_rbmap_size(void const *map);
_Bool scc_rbmap_empty(void const *map);
void scc_rbmap_free(void *map);

static inline void const *scc_rbmnode_key(
    struct scc_rbtree_base const *restrict base,
    void const *restrict node
) {
    return scc_rbnode_value_qual(base, node, const);
}

static inline void *scc_rbmnode_value(
    struct scc_rbtree_base const *restrict base,
    void *restrict node,
    size_t valoff
) {
    return (unsigned char *)scc_rbnode_value(base, node) + valoff;
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
