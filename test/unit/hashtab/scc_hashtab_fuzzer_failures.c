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

/* test_scc_hashtab_fuzzer_failure3
 *
 * Fourth failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure3(void) {
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

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
    }

    unsigned const *elem;
    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(scc_arrsize(data) - i - 1u, scc_hashtab_size(tab));
        for(unsigned j = i + 1u; j < scc_arrsize(data); ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
            TEST_ASSERT_FALSE(scc_hashtab_insert(&tab, data[j]));
        }
    }
    scc_hashtab_free(tab);
}
/* test_scc_hashtab_fuzzer_failure4
 *
 * Fifth failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure4(void) {
    static unsigned const data[] = {
        873336334, 421127063, 451090713, 921102,
        4, 67108864, 3705922425, 4294967245,
        690563354, 908714786, 3809092362, 235326940,
        3171815139, 3823362868, 171172835, 67108899,
        0, 2030305280, 3453805539, 452984831,
        573122857, 171125219, 3705866787, 3809085133,
        1851837916, 1802, 690563328, 690613289,
        235802221, 434345780, 3810072857, 58339,
        3554927481, 174982861, 690563338, 858383145,
        1845939676, 688549161, 3811125545, 4293125929,
        4294967295, 235802337, 244788278, 14,
        3823307327, 3823363043, 227, 3823335680,
        1845939667, 690555402, 702753065, 170068531,
        114154723, 690555502, 174926121, 690563369,
        3823577571, 702797353, 171959271, 3823304931,
        1776494889, 2346460, 1024, 235802112,
        3888720355, 4292268003, 690559743, 702751273,
        170068533, 4080149277, 235856650, 379008014,
        3823560422, 4293125091, 16777215, 4294967040,
        235856383, 4293125119, 249692159, 741740046,
        921239, 504233984, 9909774, 870520857,
        8970, 691266338, 235545386, 690556430,
        4294960103, 3791650815, 906890766, 235837228,
        1056964608, 3823362826, 1071900457, 690559528,
        434562790, 870521059, 690607737, 3878282025,
        3926540259, 4194538, 2048, 3941263872,
        3941264106, 15526634, 14876672, 3808428032
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

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, data[i]));
        TEST_ASSERT_EQUAL_UINT64(scc_arrsize(data) - i - 1u, scc_hashtab_size(tab));
        for(unsigned j = i + 1u; j < scc_arrsize(data); ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
            TEST_ASSERT_FALSE(scc_hashtab_insert(&tab, data[j]));
        }
    }
    scc_hashtab_free(tab);
}
