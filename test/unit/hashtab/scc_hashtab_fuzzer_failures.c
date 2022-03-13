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
void test_scc_hashtab_fuzzer_failure1(void) {
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

/* test_scc_hashtab_fuzzer_failure2
 *
 * Third failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure2(void) {
    static unsigned const data[] = {
        873336334,  421127063,  3823311129, 668387,
        4,          67108864,   3705922425, 4294967245,
        690563354,  858383138,  3809092362, 235326940,
        235802339,  3823362868, 171172835,  67108899,
        0,          772014080,  3453806307, 452984831,
        573122857,  171125219,  3705866787, 3809085133,
        1851837916, 690563338,  690613289,  3822463341,
        1071900457, 3808486154, 3811125731, 432921641,
        3823362841, 2296371,    1024,       3823335684,
        4294954460, 690559743,  702751273,  170068531,
        114154723,  3453805322, 688549472,  1831414057,
        690607852,  3878282025, 3809099747, 3705922304,
        174982861,  690563369,  701951085,  702753065,
        171959271,  3823304940
    };

    scc_hashtab(unsigned) tab = scc_hashtab_init(unsigned, eq);

    unsigned const *elem;
    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashtab_size(tab));
        for(unsigned j = 0u; j <= i; ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
        }
    }

    scc_hashtab_free(tab);
}

