#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(unsigned const *)left == *(unsigned const *)right;
}


/* test_scc_hashtab_fuzzer_failure0
 *
 * First failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure0(void) {
    static unsigned const data[] = {
        420351502, 722330393, 2147341328, 77660160,
        690563338, 688549161, 3727239465, 2749835747,
        3823577571, 14879295
    };
    scc_hashtab(unsigned) tab = scc_hashtab_init(unsigned, eq);

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ul, scc_hashtab_size(tab));
        for(unsigned j = 0u; j <= i; ++j) {
            TEST_ASSERT_TRUE(scc_hashtab_find(tab, data[j]));
        }
    }
    scc_hashtab_free(tab);
}

/* test_scc_hashtab_fuzzer_failure1
 *
 * Second failure case detected by fuzzer
 */
void test_scc_hashtab_fuzz_failure1(void) {
    static unsigned const data[] = {
        873336334, 421127063, 3823311129, 227,
        67108864, 0, 2030043136, 3453805539,
        452984831, 690563369, 171125219, 3705866787,
        174982861, 673778285, 702753065, 171959271,
        3823304931
    };
    scc_hashtab(unsigned) tab = scc_hashtab_init(unsigned, eq);

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ul, scc_hashtab_size(tab));
        for(unsigned j = 0u; j <= i; ++j) {
            TEST_ASSERT_TRUE(scc_hashtab_find(tab, data[j]));
        }
    }

    unsigned const *elem;
    TEST_ASSERT_FALSE(scc_hashtab_insert(&tab, data[scc_arrsize(data) - 1u]));
    TEST_ASSERT_EQUAL_UINT32(scc_arrsize(data), scc_hashtab_size(tab));
    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        elem = scc_hashtab_find(tab, data[i]);
        TEST_ASSERT_TRUE(!!elem);
        TEST_ASSERT_EQUAL_UINT32(data[i], *elem);
    }

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(scc_arrsize(data) - i - 1ul, scc_hashtab_size(tab));
        for(unsigned j = i + 1u; j < scc_arrsize(data); ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
        }
    }
    scc_hashtab_free(tab);
}
