#include <scc/scc_btmap.h>

#include <assert.h>
#include <limits.h>

size_t scc_btmap_order(void const *btmap);
void *scc_btmap_impl_with_order(void *base, size_t coff, size_t rootoff);
size_t scc_btmap_impl_npad(void const *btmap);
size_t scc_btmap_size(void const *btmap);

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
