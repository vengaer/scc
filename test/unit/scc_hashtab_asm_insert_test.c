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

void test_hashtab_insertion_probing_single_duplicate(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(hashtab);

    TEST_ASSERT_TRUE(scc_hashtab_insert(hashtab, 13));

    unsigned long long hash = scc_hashtab_fnv1a(&(int){ 13 }, sizeof(int));
    long long probed = scc_hashtab_impl_insert_probe(base, hashtab, sizeof(int), hash);

    TEST_ASSERT_EQUAL_INT64(-1ll, probed);

    scc_hashtab_free(hashtab);
}

void test_hashtab_insertion_probing_single_non_duplicate(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(hashtab);

    unsigned long long hash = scc_hashtab_fnv1a(&(int){ 88 }, sizeof(int));
    unsigned slot = hash & (scc_hashtab_capacity(hashtab) - 1u);
    scc_hashtab_metatype *md = scc_hashtab_inspect_md(hashtab);
    md[slot] = 0x8000;
    long long probed = scc_hashtab_impl_insert_probe(base, hashtab, sizeof(int), hash);

    TEST_ASSERT_EQUAL_INT64(slot + 1ll, probed);
    scc_hashtab_free(hashtab);
}

void test_hashtab_insertion_probing_vacated(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(tab);

    TEST_ASSERT_TRUE(scc_hashtab_insert(tab, 12));
    unsigned long long hash = scc_hashtab_fnv1a(&(int){ 12 }, sizeof(int));
    unsigned slot = hash & (base->ht_capacity - 1u);
    scc_hashtab_metatype *md = scc_hashtab_inspect_md(tab);
    md[slot] |= SCC_HASHTAB_INSPECT_VACATED;
    long long probed = scc_hashtab_impl_insert_probe(base, tab, sizeof(int), hash);
    TEST_ASSERT_EQUAL_INT64(slot + 0ll, probed);
    scc_hashtab_free(tab);
}

#endif