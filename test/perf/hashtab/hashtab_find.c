#include "hashtab_find.h"

#include <scc/scc_hashtab.h>

static bool eq(void const *left, void const *right) {
    return *(bm_type const *)left == *(bm_type const *)right;
}

void *hashtab_find_init(void) {
    scc_hashtab(bm_type) tab = scc_hashtab_init(bm_type, eq);
    if(!scc_hashtab_reserve(tab, scc_hashtab_capacity(tab) << 1u)) {
        return 0;
    }

    return tab;
}
void hashtab_find_free(void *tab) { scc_hashtab_free(tab);
}

void hashtab_find_insert_all(void **table, bm_type const *data, size_t size) {
    scc_hashtab(bm_type) tab = *table;
    scc_hashtab_reserve(tab, size << 1u);
    for(unsigned i = 0u; i < size; ++i) {
        scc_hashtab_insert(tab, data[i]);
    }
    *table = tab;
}

void hashtab_find_clear(void *table) {
    scc_hashtab(bm_type) tab = table;
    scc_hashtab_clear(tab);
}

bool hashtab_find_benchmark(void *handle, bm_type const *data, size_t size) {
    scc_hashtab(bm_type) tab = handle;
    bool all_found = true;
    for(unsigned i = 0u; i < size; ++i) {
        all_found &= !!scc_hashtab_find(tab, data[i]);
    }
    return all_found;
}
