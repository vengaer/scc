#include <scc/svec.h>

void test_scc_svec_pop_safe_panic(void) {
    int *svec = scc_svec_new(int);
    scc_svec_pop_safe(svec);
    scc_svec_free(svec);
}

void test_scc_svec_at_panic(void) {
    int *svec = scc_svec_from(int, 1, 2, 3);
    (void)scc_svec_at(svec, 3);
    scc_svec_free(svec);
}
