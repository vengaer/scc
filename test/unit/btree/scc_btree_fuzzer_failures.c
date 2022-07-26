#include <inspect/scc_btree_inspect.h>
#include <scc/scc_btree.h>
#include <scc/scc_mem.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

void test_scc_btree_fuzzer_failure0(void) {
    static int const data[] = {
        0,   72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  44,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   118, 118, 118, 118, 118

    };
    scc_btree(int) btree = scc_btree_with_order(int, compare, 10);

    int const *p;
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
        for(size_t j = 0u; j < i; ++j) {
            p = scc_btree_find(btree, data[j]);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(*p, data[j]);
        }
    }
    scc_btree_free(btree);
}
