#include "hashtab_find.h"

#include <scc/hashtab.h>

static bool eq(void const *left, void const *right) {
    return *(bm_type const *)left == *(bm_type const *)right;
}

void *hashtab_find_init(void) {
    scc_hashtab(bm_type) tab = scc_hashtab_new(bm_type, eq);
    if(!scc_hashtab_reserve(&tab, scc_hashtab_capacity(tab) << 1u)) {
        return 0;
    }

    return tab;
}
void hashtab_find_free(void *tab) {
    scc_hashtab_free(tab);
}

void hashtab_find_insert_all(void **tab, bm_type const *data, size_t size) {
    scc_hashtab_reserve((scc_hashtab(bm_type) *)tab, size << 1u);
    for(unsigned i = 0u; i < size; ++i) {
        scc_hashtab_insert((scc_hashtab(bm_type) *)tab, data[i]);
    }
}

void hashtab_find_clear(void *tab) {
    scc_hashtab_clear(tab);
}

bool hashtab_find_benchmark(void *tab, bm_type const *data, size_t size) {
    bool all_found = true;
    for(unsigned i = 0u; i < size; ++i) {
        all_found &= !!scc_hashtab_find((scc_hashtab(bm_type))tab, data[i]);
    }
    return all_found;
}
