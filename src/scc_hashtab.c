#include <scc/scc_hashtab.h>

#include <assert.h>
#include <limits.h>

void *scc_hashtab_impl_init(void *inittab, scc_eq eq, size_t dataoff, size_t mdoff, size_t capacity) {
    struct scc_hashtab *tab = inittab;
    tab->ht_eq = eq;
    tab->ht_mdoff = mdoff;
    tab->ht_size = 0u;
    tab->ht_capacity = capacity;
    tab->ht_dynalloc = 0;

    size_t const off = dataoff - offsetof(struct scc_hashtab, ht_fwoff) - sizeof(tab->ht_fwoff);
    assert(off < UCHAR_MAX);
    tab->ht_fwoff = (unsigned char)off;

    unsigned char *ht_tmp = (unsigned char *)inittab + dataoff;
    ht_tmp[-1] = (unsigned char)off;
    return ht_tmp;
}
