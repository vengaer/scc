#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>

#include <stdbool.h>
#include <stddef.h>

#include <unity.h>

#ifndef SCC_SIMD_ISA_UNSUPPORTED
extern long long scc_hashtab_impl_insert_probe(
    struct scc_hashtab *base,
    void *tab,
    size_t elemsize,
    unsigned long long hash
);

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

void test_hashtab_insertion_probing_when_empty(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(hashtab);

    int value = 13;
    unsigned long long hash = scc_hashtab_fnv1a(&value, sizeof(value));
    unsigned slot = hash & (scc_hashtab_capacity(hashtab) - 1u);
    long long probed = scc_hashtab_impl_insert_probe(base, hashtab, sizeof(value), hash);
    TEST_ASSERT_EQUAL_INT64((long long)slot, probed);

    scc_hashtab_free(hashtab);
}


#endif
