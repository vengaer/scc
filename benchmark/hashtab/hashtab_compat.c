#include "hashtab_compat.h"

#include <instrumentation/types.h>

#include <scc/hashtab.h>

int eq(void const *l, void const *r) {
    return *(bm_type const *)l == *(bm_type const *)r;
}

void *hashtab_new(void) {
    return scc_hashtab_new_dyn(bm_type, eq);
}

void hashtab_free(void *hashtab) {
    scc_hashtab_free(hashtab);
}
