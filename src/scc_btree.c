#include <scc/scc_btree.h>

#include <assert.h>
#include <limits.h>

size_t scc_btree_impl_npad(void const *btree);
size_t scc_btree_order(void const *btree);
size_t scc_btree_size(void const *btree);

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

void *scc_btree_impl_new(void *base, size_t coff) {
#define base ((struct scc_btree_base *)base)
    size_t fwoff = coff - offsetof(struct scc_btree_base, bt_fwoff) - sizeof(base->bt_fwoff);
    assert(fwoff <= UCHAR_MAX);
    base->bt_fwoff = (unsigned char)fwoff;
    unsigned char *btree = (unsigned char *)base + coff;
    scc_btree_set_bkoff(btree, fwoff);
    return btree;
#undef base
}

void scc_btree_free(void *btree) {
    struct scc_btree_base *base = scc_btree_impl_base(btree);
    scc_arena_release(&base->bt_arena);
}
