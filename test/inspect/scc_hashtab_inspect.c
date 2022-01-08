#include "scc_hashtab_inspect.h"

#include <scc/scc_mem.h>

struct scc_hashtab *scc_hashtab_inspect_base(void *tab) {
    struct scc_hashtab *base = scc_container(
        tab - ((unsigned char *)tab)[-1] - sizeof(base->ht_fwoff),
        struct scc_hashtab,
        ht_fwoff
    );
    return base;
}

scc_hashtab_metatype *scc_hashtab_inspect_md(void *tab) {
    struct scc_hashtab *base = scc_hashtab_inspect_base(tab);
    return (void *)((unsigned char *)base + base->ht_mdoff);
}

void *scc_hashtab_inspect_impl_data(void *tab, size_t elemsize) {
    return (unsigned char *)tab + elemsize;
}
