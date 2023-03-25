#include <inspect/rbtree_inspect.h>
#include <scc/mem.h>
#include <scc/rbmap.h>

#include <unity.h>

#ifdef SCC_MUTATION_TEST
enum { TEST_SIZE = 64 };
#else
enum { TEST_SIZE = 4096 };
#endif

static int compare(void const *left, void const *right) {
    return *(int const *)left - *(int const *)right;
}

void test_scc_rbmap_new_dyn(void) {
    scc_rbmap(int, int) rbmap = scc_rbmap_new_dyn(int, int, compare);
    struct scc_rbtree_base *base = scc_rbtree_impl_base(rbmap);
    TEST_ASSERT_TRUE(base->rb_dynalloc);
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_insert(void) {
    scc_inspect_mask mask;
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_rbmap_size(rbmap));
        mask = scc_rbtree_inspect_properties(rbmap);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_find(void) {
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);

    scc_inspect_mask mask;
    int *val;
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, i));
        val = scc_rbmap_find(rbmap, i);
        TEST_ASSERT_TRUE(!!val);
        TEST_ASSERT_EQUAL_INT32(i, *val);
        *val = i << 1;

        for(int j = 0; j < i; ++j) {
            val = scc_rbmap_find(rbmap, j);
            TEST_ASSERT_EQUAL_INT32(2 * j, *val);
        }
        mask = scc_rbtree_inspect_properties(rbmap);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_remove(void) {
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, 0));
    }

    scc_inspect_mask mask;
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_rbmap_remove(rbmap, i));
        TEST_ASSERT_FALSE(scc_rbmap_remove(rbmap, i));
        mask = scc_rbtree_inspect_properties(rbmap);
        TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);
    }
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_foreach(void) {
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);

    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, i));
    }

    scc_rbmap_iter(int, int) iter;
    int i = 0;
    scc_rbmap_foreach(iter, rbmap) {
        TEST_ASSERT_EQUAL_INT32(iter->key, i);
        TEST_ASSERT_EQUAL_INT32(iter->value, i);
        iter->value += iter->value;
        ++i;
    }

    i = 0;
    scc_rbmap_foreach(iter, rbmap) {
        TEST_ASSERT_EQUAL_INT32(iter->key, i);
        TEST_ASSERT_EQUAL_INT32(iter->value, i + i);
        ++i;
    }
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_foreach_reversed(void) {
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);

    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, i, i));
    }

    scc_rbmap_iter(int, int) iter;
    int i = TEST_SIZE - 1;
    scc_rbmap_foreach_reversed(iter, rbmap) {
        TEST_ASSERT_EQUAL_INT32(i, iter->key);
        TEST_ASSERT_EQUAL_INT32(i, iter->value);
        iter->value = 0;
        --i;
    }

    i = TEST_SIZE - 1;

    scc_rbmap_foreach_reversed(iter, rbmap) {
        TEST_ASSERT_EQUAL_INT32(i--, iter->key);
        TEST_ASSERT_FALSE(!!iter->value);
    }
    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_update_existing(void) {
    scc_rbmap(int, int) rbmap = scc_rbmap_new(int, int, compare);

    TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, 1, 1));
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_rbmap_size(rbmap));

    TEST_ASSERT_TRUE(scc_rbmap_insert(&rbmap, 1, 2));
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_rbmap_size(rbmap));

    int *val = scc_rbmap_find(rbmap, 1);
    TEST_ASSERT_TRUE(!!val);
    TEST_ASSERT_EQUAL_INT32(2, *val);

    scc_rbmap_free(rbmap);
}

void test_scc_rbmap_clone(void) {
    scc_rbmap(int, int) orig = scc_rbmap_new(int, int, compare);

    for(int i = 222; i; --i) {
        TEST_ASSERT_TRUE(scc_rbmap_insert(&orig, i, -1));
    }
    scc_rbmap(int, int) new = scc_rbmap_clone(orig);
    TEST_ASSERT_TRUE(!!new);

    scc_inspect_mask mask = scc_rbtree_inspect_properties(new);
    TEST_ASSERT_EQUAL_UINT64(mask & SCC_RBTREE_ERR_MASK, 0ull);

    scc_rbmap_iter(int, int) iter;
    int const *p;
    scc_rbmap_foreach(iter, orig) {
        p = scc_rbmap_find(new, iter->key);
        TEST_ASSERT_TRUE(!!p);
        TEST_ASSERT_EQUAL_INT32(iter->value, *p);
    }

    scc_rbmap_foreach(iter, new) {
        p = scc_rbmap_find(orig, iter->key);
        TEST_ASSERT_TRUE(!!p);
        TEST_ASSERT_EQUAL_INT32(iter->value, *p);
    }

    scc_rbmap_foreach_reversed(iter, new) {
        p = scc_rbmap_find(orig, iter->key);
        TEST_ASSERT_TRUE(!!p);
        TEST_ASSERT_EQUAL_INT32(iter->value, *p);
    }

    scc_rbmap_free(orig);
    scc_rbmap_free(new);
}
