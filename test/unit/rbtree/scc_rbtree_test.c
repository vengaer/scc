#include <inspect/rbtree_inspect.h>
#include <scc/mem.h>
#include <scc/rbtree.h>

#include <unity.h>

#ifdef SCC_MUTATION_TEST
enum { TEST_SIZE = 64 };
#else
enum { TEST_SIZE = 4096 };
#endif

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

void test_scc_rbtree_clone(void) {
    scc_rbtree(int) orig = scc_rbtree_new(int, compare);

    for(int i = 0; i < 312; ++i) {
        TEST_ASSERT_TRUE(scc_rbtree_insert(&orig, i));
    }

    scc_rbtree(int) new = scc_rbtree_clone(orig);
    TEST_ASSERT_TRUE(!!new);

    TEST_ASSERT_EQUAL_UINT64(scc_rbtree_size(orig), scc_rbtree_size(new));
    unsigned long long status = scc_rbtree_inspect_properties(new);
    TEST_ASSERT_EQUAL_UINT64(0ull, status & SCC_RBTREE_ERR_MASK);

    int const *iter;
    int const *p;
    scc_rbtree_foreach(iter, orig) {
        p = scc_rbtree_find(new, *iter);
        TEST_ASSERT_TRUE(!!p);
        TEST_ASSERT_EQUAL_INT32(*iter, *p);
    }

    scc_rbtree_foreach(iter, new) {
        p = scc_rbtree_find(orig, *iter);
        TEST_ASSERT_TRUE(!!p);
        TEST_ASSERT_EQUAL_INT32(*iter, *p);
    }

    scc_rbtree_foreach_reversed(iter, new) {
        p = scc_rbtree_find(orig, *iter);
        TEST_ASSERT_TRUE(!!p);
        TEST_ASSERT_EQUAL_INT32(*iter, *p);
    }

    scc_rbtree_free(orig);
    scc_rbtree_free(new);
}
