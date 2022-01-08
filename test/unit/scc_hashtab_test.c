#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static unsigned long long ident(void const *data, size_t size) {
    switch(size) {
        case sizeof(unsigned char):
            return *(unsigned char const *)data;
        case sizeof(unsigned short):
            return *(unsigned short const *)data;
        case sizeof(unsigned):
            return *(unsigned const *)data;
        case sizeof(unsigned long long):
            return *(unsigned long long const *)data;
        default:
            TEST_ASSERT_EQUAL_UINT64(0ull, size);
    }
    return 0ull;
}

void test_scc_hashtab_bkoff(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(hashtab);
    /* Probably UB... */
    TEST_ASSERT_EQUAL_PTR(base->ht_eq, eq);
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_fwoff(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(hashtab);
    size_t const off = base->ht_fwoff + offsetof(struct scc_hashtab, ht_fwoff) + sizeof(base->ht_fwoff);
    int *fwp = (void *)((unsigned char *)base + off);
    TEST_ASSERT_EQUAL_PTR(hashtab, fwp);
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_explicit_hash(void) {
    scc_hashtab(int) hashtab = scc_hashtab_with_hash(int, eq, ident);
    struct scc_hashtab *base = scc_hashtab_inspect_base(hashtab);
    /* UB */
    TEST_ASSERT_EQUAL_PTR(base->ht_hash, ident);
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_guard_initialized(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab *base = scc_hashtab_inspect_base(hashtab);
    scc_hashtab_metatype *guard =
        (void *)((unsigned char *)base + base->ht_mdoff + base->ht_capacity * sizeof(*guard));

    for(unsigned i = 0u; i < scc_hashtab_impl_guardsz(); ++i) {
        TEST_ASSERT_EQUAL_UINT16(0x4000u, guard[i]);
    }

    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_insert_single(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    TEST_ASSERT_TRUE(scc_hashtab_insert(hashtab, 13));
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_insert_disallow_duplicate(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    TEST_ASSERT_TRUE(scc_hashtab_insert(tab, 88));
    TEST_ASSERT_FALSE(scc_hashtab_insert(tab, 88));
    scc_hashtab_free(tab);
}

void test_scc_hashtab_insert_10x(void) {
    enum { SIZE = 10 };
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    for(int i = 0; i < SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashtab_size(tab));
    }
    scc_hashtab_free(tab);
}

void test_scc_hashtab_insert_rehash(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    size_t const cap = scc_hashtab_capacity(tab);
    for(unsigned i = 0; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashtab_size(tab));
    }
    TEST_ASSERT_NOT_EQUAL_UINT64(cap, scc_hashtab_capacity(tab));
    scc_hashtab_free(tab);
}
