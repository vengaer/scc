#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

#ifndef SCC_SIMD_ISA_UNSUPPORTED

extern long long scc_hashtab_impl_find_probe(
    struct scc_hashtab const *base,
    void const *tab,
    unsigned long long elemsize,
    unsigned long long hash
);

void test_scc_hashtab_find_probe_empty(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash = base->ht_hash(&(int){ 88 }, sizeof(int));
    long long index = scc_hashtab_impl_find_probe(base, tab, sizeof(int), hash);
    TEST_ASSERT_EQUAL_INT64(-1ll, index);
    scc_hashtab_free(tab);
}

#endif
