#include <inspect/scc_btree_inspect.h>
#include <scc/scc_btmap.h>
#include <scc/scc_mem.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btmap_new(void) {
    scc_btmap(int, int) btmap = scc_btmap_new(int, int, compare);
    scc_btmap_free(btmap);
}
