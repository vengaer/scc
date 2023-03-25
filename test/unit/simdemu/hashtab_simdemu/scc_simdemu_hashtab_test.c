#include <inspect/hashtab_inspect.h>

#include <scc/arch.h>
#include <scc/hashtab.h>
#include <scc/swvec.h>

#include <stdbool.h>
#include <string.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static unsigned long long two(void const *data, size_t len) {
    (void)data;
    (void)len;
    return 2u;
}

static unsigned long long seventeen(void const *data, size_t len) {
    (void)data;
    (void)len;
    return 17u;
}

#ifdef SCC_SIMD_ISA
extern int scc_simd_support;
static int simd_backup;

static void disable_simd(void) {
    simd_backup = scc_simd_support;
    scc_simd_support = 0;
}

static void restore_simd(void) {
    scc_simd_support = simd_backup;
}
#else
#define disable_simd() (void)0
#define restore_simd() (void)0
#endif

static unsigned long long ident(void const *data, size_t len) {
    (void)len;
    return *(int const *)data;
}

void test_simdemu_hashtab_simple_find_insert_probe(void) {
    disable_simd();
    static int const data[] = {
        1, 4, 8, 10, 14, 17, 21, 25
    };
    scc_hashtab(int) tab = scc_hashtab_with_hash(int, eq, ident);

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
    }

    TEST_ASSERT_EQUAL_UINT64(SCC_HASHTAB_STACKCAP, scc_hashtab_capacity(tab));

    int const *p;
    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        p = scc_hashtab_find(tab, data[i]);
        TEST_ASSERT_TRUE(!!p);
        TEST_ASSERT_EQUAL_INT32(data[i], *p);
        /* Identity without overlap guarantees hashing to slot where
         * offset is the actual value */
        TEST_ASSERT_EQUAL_PTR((int const *)scc_hashtab_inspect_data(tab) + data[i], p);
    }
    scc_hashtab_free(tab);
    restore_simd();
}

void test_simdemu_hashtab_probe_on_rehash(void) {
    disable_simd();
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    size_t const cap = scc_hashtab_capacity(tab);
    int const *p;
    for(unsigned i = 0; i < 2 * cap; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, (int)i));

        for(unsigned j = 0u; j < i; ++j) {
            p = scc_hashtab_find(tab, (int)j);
            TEST_ASSERT_TRUE(!!p);
            TEST_ASSERT_EQUAL_INT32(j, *p);
        }
    }
    scc_hashtab_free(tab);
    restore_simd();
}

void test_scc_simdemu_hashtab_kill_find_mutants(void) {
    disable_simd();
    scc_hashtab(int) tab = scc_hashtab_with_hash(int, eq, two);
    struct scc_hashtab_base *base = scc_hashtab_impl_base(tab);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);

    base->ht_size = 1;
    memset(md, ~(((unsigned char)~0) >> 1u), scc_hashtab_capacity(tab) + SCC_HASHTAB_GUARDSZ);

    TEST_ASSERT_FALSE(!!scc_hashtab_find(tab, 32));

    int *data = scc_hashtab_inspect_data(tab);
    data[1] = 32;

    int const *p = scc_hashtab_find(tab, 32);
    TEST_ASSERT_TRUE(!!p);
    TEST_ASSERT_EQUAL_PTR(data + 1u, p);

    scc_hashtab_free(tab);
    restore_simd();
}

void test_scc_simdemu_hashtab_insert_residual(void) {
    disable_simd();
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    memset(md, ~(((unsigned char)~0) >> 1u), scc_hashtab_capacity(tab) + SCC_HASHTAB_GUARDSZ - 1u);
    md[1] = 0;

    /* Should not cause infinite loop */
    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, 1));

    int *data = scc_hashtab_inspect_data(tab);
    TEST_ASSERT_EQUAL_INT32(1, data[1]);

    scc_hashtab_free(tab);
    restore_simd();
}

void test_scc_simdemu_hashtab_insert_first(void) {
    disable_simd();
    scc_hashtab(int) tab = scc_hashtab_with_hash(int, eq, seventeen);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    md[17] = ~(((unsigned char)~0) >> 1u);
    int *data = scc_hashtab_inspect_data(tab);

    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, 18));
    TEST_ASSERT_EQUAL_INT32(18, data[18]);

    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, 19));
    TEST_ASSERT_EQUAL_INT32(19, data[19]);

    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, 20));
    TEST_ASSERT_EQUAL_INT32(20, data[20]);

    TEST_ASSERT_FALSE(scc_hashtab_insert(&tab, 20));

    restore_simd();
}
