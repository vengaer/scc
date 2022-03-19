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

/* test_scc_hashtab_fuzzer_failure5
 *
 * Sixth failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure5(void) {
    static unsigned const data[] = {
        4278910730, 2382594815, 19821101,   5267712,
        4197131546, 4210689177, 4294965940, 656932655,
        656877351,  262144,     235798528,  3374965518,
        3603948519, 131071,     0,          689569792,
        3810666793, 587871785,  841161994,  249760499,
        4211133916, 735503617,  371337676,  171125219,
        14879267,   168218076,  1624104163, 690555502,
        703342889,  3593039145, 4294912483, 4294967295,
        171959271,  3823304931, 3306032681, 3810270236,
        587871203,  3816358912, 4291681507, 3605332479,
        573164587,  3705866787, 3809085133, 1851837916,
        690563338,  690613289,  3822463341, 1072424745,
        3808486154, 3811125731, 600693801,  2536771086,
        858383894,  170068707,  114154723,  4194304,
        3823363043, 3705866979, 175005901,  690563369,
        1831414252, 517163,     24944,      702753065,
        2346460,    1024,       234885888,  235011854,
        3888720355, 4292268003, 511,        690559488,
        702751273,  170068534,  4080147747, 235856650,
        379008014,  235802342,  434345780,  1024388275,
        3505443024, 1261076521, 1261076547, 4210751555,
        3124427514, 3823307514, 262184,     3816357888,
        4080198357, 662241280,  767622439,  2539678976,
        4210690560, 184187642,  4076883706, 421126912,
        3823311129, 227,        2277424830, 190,
        3810066553, 171107299,  2197750784, 2382619406,
        767622514,  4210688979, 4281335807, 4292551621
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

/* test_scc_hashtab_fuzzer_failure6
 *
 * Seventh failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure6(void) {
    static unsigned const data[] = {
        435286510, 170, 4294967081
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

/* test_scc_hashtab_fuzzer_failure7
 *
 * Eighth failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure7(void) {
    static unsigned const data[] = {
        873336334,  421061527,  3823311129, 668387,
        4,          69074944,   3705922425, 4294967245,
        703275561,  170068531,  114154723,  235856654,
        3818337294, 421075225,  870573027,  8970,
        973078528,  789248762,  587140554,  171125219,
        4280093219, 33554431,   3677436828, 841210843,
        249760499,  2536771086, 421075427,  3823362841,
        2296371,    1024,       0,          3812260100,
        4294954460, 623191551,  690559525,  3688571433,
        3688618971, 702742747,  870523658,  114154506,
        3453805322, 688549472,  1831414057, 690607852,
        3878282025, 171901155,  3705922531, 174982861,
        690563369,  701951085,  702753065,  171959271,
        1162167788, 14934784,   84215046,   3676554501,
        2161837311
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

/* test_scc_hashtab_fuzzer_failure8
 *
 * Ninth failure case detected by fuzzer
 */
void test_scc_hashtab_fuzzer_failure8(void) {
    static unsigned const data[] = {
        102170378,  872418062,  690563369,  493,
        434341904,  689510681,  14935011,   0,
        3200122880, 12486590,   7929856,    2044950788,
        3453805539, 452984831,  573122857,  536871139,
        247381504,  2536967694, 421075427,  3810124771,
        690618367,  908714786,  3943310090, 235326940,
        235802339,  471604421,  234881024,  702746126,
        3487819721, 33619926,   436207616,  134217863,
        589105699,  3809145650, 873336334,  702944919,
        587916083,  3453805322, 3705866758, 175005901,
        690563371,  1831414252, 517163,     24944,
        170517475,  235278079,  691273739,  1831414057,
        452984577,  3823312099, 2296371,    3693732649,
        9165,       3808428036, 171321600,
    };
    scc_hashtab(unsigned) tab = scc_hashtab_init(unsigned, eq);

    unsigned const *elem;
    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, data[i]));
        for(unsigned j = 0u; j <= i; ++j) {
            TEST_ASSERT_FALSE(scc_hashtab_insert(&tab, data[j]));
        }
    }

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, data[i]));
        for(unsigned j = i + 1u; j < scc_arrsize(data); ++j) {
            elem = scc_hashtab_find(tab, data[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT32(data[j], *elem);
            TEST_ASSERT_FALSE(scc_hashtab_insert(&tab, data[j]));
        }
    }
    scc_hashtab_free(tab);
}

