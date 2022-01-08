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

void test_scc_hashtab_find_single_collision(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash = base->ht_hash(&(int){ 231 }, sizeof(int));
    scc_hashtab_metatype *md = scc_hashtab_inspect_md(tab);

    /* "insert" dummy element at position where 231 would be inserted */
    unsigned slot = hash & (base->ht_capacity - 1u);
    md[slot] = (~(hash >> 50) & 0x3fffu) | SCC_HASHTAB_INSPECT_OCCUPIED;

    TEST_ASSERT_TRUE(scc_hashtab_insert(tab, 231));

    long long index = scc_hashtab_impl_find_probe(base, tab, sizeof(int), hash);
    TEST_ASSERT_EQUAL_INT64(slot + 1ll, index);

    scc_hashtab_free(tab);
}

void test_scc_hashtab_find_single_vacated_collision(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash = base->ht_hash(&(int){ 7777 }, sizeof(int));
    scc_hashtab_metatype *md = scc_hashtab_inspect_md(tab);

    /* "insert" at hashed position */
    unsigned slot = hash & (base->ht_capacity - 1u);
    md[slot] = (~(hash >> 50) & 0x3fffu) | SCC_HASHTAB_INSPECT_OCCUPIED;

    TEST_ASSERT_TRUE(scc_hashtab_insert(tab, 7777));

    /* "remove" at hashed position */
    md[slot] |= SCC_HASHTAB_INSPECT_VACATED;

    long long index = scc_hashtab_impl_find_probe(base, tab, sizeof(int), hash);
    /* Find should still detect element after removal of colliding one */
    TEST_ASSERT_EQUAL_INT64(slot + 1ll, index);
    scc_hashtab_free(tab);
}

#endif
