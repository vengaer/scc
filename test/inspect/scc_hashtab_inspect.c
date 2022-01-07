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

