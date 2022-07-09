#include <scc/scc_vec.h>

void test_scc_vec_pop_safe_panic(void) {
    int *vec = scc_vec_new(int);
    scc_vec_pop_safe(vec);
    scc_vec_free(vec);
}

void test_scc_vec_at_panic(void) {
    int *vec = scc_vec_from(int, 1, 2, 3);
    (void)scc_vec_at(vec, 3);
    scc_vec_free(vec);
}
