#include <scc/scc_btree.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btree_init(void) {
    scc_btree(int) btree = scc_btree_init(int, compare);
    scc_btree_free(btree);
}
