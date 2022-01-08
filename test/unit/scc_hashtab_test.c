#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static bool ueq(void const *left, void const *right) {
    return *(unsigned const *)left == *(unsigned const *)right;
}

static bool contains(int *tab, int value) {
    int *data = scc_hashtab_inspect_data(tab);
    for(unsigned i = 0u; i < scc_hashtab_capacity(tab); ++i) {
        if(data[i] == value) {
            return true;
        }
    }
    return false;
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
    TEST_ASSERT_TRUE(contains(hashtab, 13));
    scc_hashtab_free(hashtab);
}

void test_scc_hashtab_insert_disallow_duplicate(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    TEST_ASSERT_TRUE(scc_hashtab_insert(tab, 88));
    TEST_ASSERT_TRUE(contains(tab, 88));
    TEST_ASSERT_FALSE(scc_hashtab_insert(tab, 88));
    TEST_ASSERT_TRUE(contains(tab, 88));
    scc_hashtab_free(tab);
}

void test_scc_hashtab_insert_10x(void) {
    enum { SIZE = 10 };
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    for(int i = 0; i < SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashtab_size(tab));
        for(int j = 0u; j <= i; ++j) {
            TEST_ASSERT_TRUE(contains(tab, j));
        }
    }
    scc_hashtab_free(tab);
}

void test_scc_hashtab_insert_rehash(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    size_t const cap = scc_hashtab_capacity(tab);
    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashtab_size(tab));
        for(unsigned j = 0u; j <= i; ++j) {
            TEST_ASSERT_TRUE(contains(tab, j));
        }
    }
    TEST_ASSERT_NOT_EQUAL_UINT64(cap, scc_hashtab_capacity(tab));
    scc_hashtab_free(tab);
}

void test_scc_hashtab_trivial_find(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    TEST_ASSERT_TRUE(scc_hashtab_insert(tab, 38));
    int const *val = scc_hashtab_find(tab, 38);
    TEST_ASSERT_TRUE(val);
    TEST_ASSERT_EQUAL_INT32(38, *val);
    scc_hashtab_free(tab);
}

void test_scc_hashtab_non_rehashed_find(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    size_t const cap = scc_hashtab_capacity(tab);
    for(unsigned i = 0u; i < 10u; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, i));
    }
    TEST_ASSERT_EQUAL_UINT64(cap, scc_hashtab_capacity(tab));
    int const *elem;
    for(unsigned i = 0u; i < scc_hashtab_size(tab); ++i) {
        elem = scc_hashtab_find(tab, i);
        TEST_ASSERT_TRUE(elem);
        TEST_ASSERT_EQUAL_INT32(i, *elem);
    }
    scc_hashtab_free(tab);
}

void test_scc_hashtab_rehashed_find(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    size_t const cap = scc_hashtab_capacity(tab);
    int const *elem;
    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, i));
        for(unsigned j = 0u; j < i; ++j) {
            elem = scc_hashtab_find(tab, i);
            TEST_ASSERT_TRUE(elem);
            TEST_ASSERT_EQUAL_INT32(i, *elem);
        }
    }
    scc_hashtab_free(tab);
}

void test_scc_hashtab_removal(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    TEST_ASSERT_TRUE(scc_hashtab_insert(tab, 13));
    TEST_ASSERT_TRUE(scc_hashtab_find(tab, 13));
    TEST_ASSERT_TRUE(scc_hashtab_remove(tab, 13));
    TEST_ASSERT_FALSE(scc_hashtab_find(tab, 13));
    scc_hashtab_free(tab);
}

void test_scc_hashtab_fuzzer_failure_case0(void) {
    scc_hashtab(unsigned) tab = scc_hashtab_init(unsigned, ueq);
    static unsigned const data[] = {
        3394184074u, 3621246897
    };
    unsigned const *elem;
    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, data[i]));
        for(unsigned j = 0u; j <= i; ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
        }
    }
    scc_hashtab_free(tab);
}

void testscc_hashtab_fuzzer_failure_case1(void) {
    scc_hashtab(unsigned) tab = scc_hashtab_init(unsigned, ueq);
    static unsigned const data[] = {
        4278910730u, 2382594815u, 767622536u,  1617063168u,
        4194305175u, 3321559802u, 1912372959u, 4294836082u,
        1831714816u, 9920767u,    4210752010u, 16434682u,
        4210752000u, 3678075642u, 3688618971u, 789248762u,
        17632714u
    };
    unsigned const *elem;
    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(tab, data[i]));
        for(unsigned j = 0u; j <= i; ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
        }
    }
    scc_hashtab_free(tab);
}

