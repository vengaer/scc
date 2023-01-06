#include <inspect/hashtab_inspect.h>
#include <scc/hashtab.h>
#include <simdemu/simd_swemu.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static unsigned long long ident(void const *data, size_t len) {
    (void)len;
    return *(int const *)data;
}

void test_simdemu_hashtab_simple_find_insert_probe(void) {
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
}

void test_simdemu_hashtab_probe_on_rehash(void) {
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
}
