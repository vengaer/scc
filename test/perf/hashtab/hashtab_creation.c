#include <perfutil/memory.h>
#include <scc/scc_hashtab.h>

#include <stdbool.h>

#ifndef SCC_BM_TYPE
#error SCC_BM_TYPE not defined
#endif

typedef SCC_BM_TYPE bm_type;

static bool eq(void const *left, void const *right) {
    return *(bm_type const *)left == *(bm_type const *)right;
}

void automatic_init_free_benchmark(void) {
    scc_hashtab(bm_type) tab = scc_hashtab_init(bm_type, eq);
    scc_clobber();
    scc_hashtab_free(tab);
}
