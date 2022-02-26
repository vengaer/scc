#include <scc/scc_hashtab.h>

#include "bmtypes.h"

#include <stdbool.h>
#include <stddef.h>

static bool eq(void const *left, void const *right) {
    return *(bm_type const *)left == *(bm_type const *)right;
}

void *hashtab_insertion_init(void) {
    scc_hashtab(bm_type) tab = scc_hashtab_init(bm_type, eq);
    if(!scc_hashtab_reserve(tab, scc_hashtab_capacity(tab) << 1u)) {
        return 0;
    }

    return tab;
}

void hashtab_insertion_free(void *tab) {
    scc_hashtab_free(tab);
}

bool hashtab_insertion_benchmark(void **handle, bm_type const *data, size_t size) {
    scc_hashtab(bm_type) tab = *handle;
    bool all_inserted = true;
    for(size_t i = 0u; i < size; ++i) {
        all_inserted &= !!scc_hashtab_insert(tab, data[i]);
    }
    *handle = tab;
    return all_inserted;
}
