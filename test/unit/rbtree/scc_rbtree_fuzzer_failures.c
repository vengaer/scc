#include <inspect/rbtree_inspect.h>
#include <scc/mem.h>
#include <scc/rbtree.h>

#include <unity.h>

int ucmp(void const *left, void const *right) {
    uint32_t leftval = *(unsigned const *)left;
    uint32_t rightval = *(unsigned const *)right;
    return (leftval < rightval ? -1 : 1) * !(leftval == rightval);
}

void test_scc_rbtree_removal_fuzzer_failure0(void) {
    scc_rbtree(unsigned) handle = scc_rbtree_new(unsigned, ucmp);
    TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, 160340));
    TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, 288));
    TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, 0));
    TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, 24576));
    TEST_ASSERT_TRUE(scc_rbtree_remove(handle, 160340));
    TEST_ASSERT_TRUE(scc_rbtree_remove(handle, 288));
    TEST_ASSERT_TRUE(scc_rbtree_remove(handle, 0));
    TEST_ASSERT_TRUE(scc_rbtree_remove(handle, 24576));
    scc_rbtree_free(handle);
}

void test_scc_rbtree_insert_remove_interchanged(void) {
    scc_rbtree(unsigned) handle = scc_rbtree_new(unsigned, ucmp);
    unsigned values[] = {
        4294442752, 1644167167, 2013394256, 4294963199, /* NOLINT(readability-magic-numbers) */
        16777275,   0,          4278190080, 591934463   /* NOLINT(readability-magic-numbers) */
    };

    unsigned long long mask = 0;
    for(unsigned j = 0u; j < 10u; ++j) {
        for(unsigned i = 0; i < scc_arrsize(values); ++i) {
            TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, values[i]));
            mask = scc_rbtree_inspect_properties(handle);
            TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
        }
        for(unsigned i = 0; i < scc_arrsize(values); ++i) {
            TEST_ASSERT_TRUE(scc_rbtree_remove(handle, values[i]));
            mask = scc_rbtree_inspect_properties(handle);
            TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
        }
    }

    scc_rbtree_free(handle);
}

void test_scc_rbtree_removal_fuzz_failure1(void) {
    scc_rbtree(unsigned) handle = scc_rbtree_new(unsigned, ucmp);
    unsigned values[] = {
        4294442752, 1644167167, 2013394256, 4294963199, /* NOLINT(readability-magic-numbers) */
        16777275,   0,          4278190080, 591934463   /* NOLINT(readability-magic-numbers) */
    };

    unsigned long long mask = 0;
    for(unsigned i = 0; i < scc_arrsize(values); ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, values[i]));
        mask = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }
    for(unsigned i = 0; i < scc_arrsize(values); ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, values[i]));
        mask = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }

    scc_rbtree_free(handle);
}

void test_scc_rbtree_fuzz_failure2(void) {
    scc_rbtree(unsigned) handle = scc_rbtree_new(unsigned, ucmp);
    TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, 117u));

    unsigned const *iter;
    scc_rbtree_foreach_reversed(iter, handle) {
        TEST_ASSERT_EQUAL_UINT32(117u, *iter);
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_fuzz_failure3(void) {
    scc_rbtree(unsigned) handle = scc_rbtree_new(unsigned, ucmp);
    unsigned data[] = {
         442130442,  4279435263, 2412773375, 3486502863,
         3489600255, 3472883712, 64815107,   4294954959,
         2865745871

    };

    for(unsigned i = 0; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, data[i]));
    }

    unsigned long long flags;
    for(unsigned i = 0u; i < scc_arrsize(data); i++) {
        TEST_ASSERT_EQUAL_UINT64(scc_arrsize(data) - i, scc_rbtree_size(handle));
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, data[i]));
        for(unsigned j = i + 1u; j < scc_arrsize(data); j++) {
            TEST_ASSERT_TRUE(scc_rbtree_find(handle, data[j]));
        }

        flags = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(0ull, flags & SCC_RBTREE_ERR_MASK);
    }

    for(unsigned i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_FALSE(scc_rbtree_find(handle, data[i]));
    }
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_rbtree_size(handle));

    scc_rbtree_free(handle);
}
