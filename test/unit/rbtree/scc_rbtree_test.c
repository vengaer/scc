#include <inspect/rbtree_inspect.h>
#include <scc/mem.h>
#include <scc/rbtree.h>

#include <unity.h>

int compare(void const *left, void const *right) {
    return *(int const *)left - *(int const *)right;
}

void test_scc_rbtree_new(void) {
    scc_rbtree(int) tree = scc_rbtree_new(int, compare);
    scc_rbtree_free(tree);
}

void test_scc_rbtree_size(void) {
    scc_rbtree(int) tree = scc_rbtree_new(int, compare);
    TEST_ASSERT_EQUAL_UINT64(0u, scc_rbtree_size(tree));
    scc_rbtree_free(tree);
}

void test_scc_rbtree_insert(void) {
    enum { TEST_SIZE = 1024 };

    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_rbtree_size(handle));
        for(int j = 0; j <= i; j++) {
            TEST_ASSERT_FALSE(scc_rbtree_insert(&handle, j));
        }
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_find(void) {
    enum { TEST_SIZE = 1024 };
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_rbtree_size(handle));

        for(int j = 0; j <= i; j++) {
            TEST_ASSERT_TRUE(scc_rbtree_find(handle, j));
        }
    }

    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_find(handle, i));
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_remove(void) {
    enum { TEST_SIZE = 512 };
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
    }
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, i));
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_duplicate_removal(void) {
    enum { TEST_SIZE = 512 };
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
    }
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, i));
        TEST_ASSERT_FALSE(scc_rbtree_remove(handle, i));
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_foreach(void) {
    enum { TEST_SIZE = 512 };
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
    }
    int i = 0;
    int const *iter;
    scc_rbtree_foreach(iter, handle) {
        TEST_ASSERT_EQUAL_INT32(i++, *iter);
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_foreach_reversed(void) {
    enum { TEST_SIZE = 512 };
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
    }
    int i = TEST_SIZE - 1;
    int const *iter;
    scc_rbtree_foreach_reversed(iter, handle) {
        TEST_ASSERT_EQUAL_INT32(i--, *iter);
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_properties_insertion(void) {
    enum { TEST_SIZE = 512 };
    unsigned long long status;
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
        status = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(status & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_properties_removal(void) {
    enum { TEST_SIZE = 512 };
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
    }
    unsigned long long status;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, i));
        status = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(status & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbtree_free(handle);
}

void test_scc_rbtree_properties_insertion_removal(void) {
    enum { CHUNKSIZE = 512 };
    scc_rbtree(int) handle = scc_rbtree_new(int, compare);
    unsigned long long status;
    int i = 0;
    for(;i < CHUNKSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
        status = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(status & SCC_RBTREE_ERR_MASK, 0ull);
    }
    for(;i < 2 * CHUNKSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&handle, i));
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, i - CHUNKSIZE));
        status = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(status & SCC_RBTREE_ERR_MASK, 0ull);
    }
    for(; i < 3 * CHUNKSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_remove(handle, i - CHUNKSIZE));
        status = scc_rbtree_inspect_properties(handle);
        TEST_ASSERT_EQUAL_UINT64(status & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbtree_free(handle);
}
