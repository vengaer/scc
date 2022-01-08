#include <scc/scc_hashtab.h>

#include <stdbool.h>
#include <stddef.h>

#ifndef SCC_BM_TYPE
#error SCC_BM_TYPE not defined
#endif

typedef SCC_BM_TYPE bm_type;

static bool eq(void const *left, void const *right) {
    return *(bm_type const *)left == *(bm_type const *)right;
}

void insertion_benchmark(bm_type const *data, size_t size) {
    scc_hashtab(bm_type) tab = scc_hashtab_init(bm_type, eq);
    for(size_t i = 0u; i < size; ++i) {
        scc_hashtab_insert(tab, data[i]);
    }
    scc_hashtab_free(tab);
}
