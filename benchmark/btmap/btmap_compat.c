#include "btmap_compat.h"

#include <instrumentation/types.h>

#include <scc/btmap.h>

int compare(void const *l, void const *r) {
    return *(bm_type const *)l - *(bm_type const *)r;
}

void *btmap_new(void) {
    return scc_btmap_new_dyn(bm_type, bm_type, compare);
}

void btmap_free(void *btmap) {
    scc_btmap_free(btmap);
}
