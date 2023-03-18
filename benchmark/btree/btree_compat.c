#include "btree_compat.h"

#include <instrumentation/types.h>

#include <scc/btree.h>

int compare(void const *l, void const *r) {
    return *(bm_type const *)l - *(bm_type const *)r;
}

void *btree_new(void) {
    return scc_btree_new_dyn(bm_type, compare);
}

void btree_free(void *btree) {
    scc_btree_free(btree);
}
