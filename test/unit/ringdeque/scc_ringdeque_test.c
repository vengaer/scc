#include <scc/scc_ringdeque.h>

#include <unity.h>

void test_scc_ringdeque_new(void) {
    scc_ringdeque(int) deque = scc_ringdeque_new(int);
    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_push_back(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    size_t cap = scc_ringdeque_capacity(deque);
    for(unsigned i = 0u; i < cap + 3u; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_back(&deque, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_ringdeque_size(deque));

        for(unsigned j = 0u; j < i; ++j) {
            TEST_ASSERT_EQUAL_UINT32(j, deque[j]);
        }
    }
    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_push_front(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    size_t cap = scc_ringdeque_capacity(deque);
    for(unsigned i = 0u; i < cap + 3u; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_front(&deque, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_ringdeque_size(deque));
    }

    struct scc_ringdeque_base *base = scc_ringdeque_impl_base(deque);
    for(unsigned i = base->rd_begin, j = cap + 2; i != base->rd_end; i = (i + 1u) & (base->rd_capacity - 1u), --j) {
        TEST_ASSERT_EQUAL_UINT32(j, deque[i]);
    }

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_pop_back(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    size_t const cap = 2 * scc_ringdeque_capacity(deque);
    unsigned i;
    for(i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_back(&deque, i));
    }

    while(!scc_ringdeque_empty(deque)) {
        TEST_ASSERT_EQUAL_UINT32(--i, scc_ringdeque_pop_back(deque));
    }

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_pop_front(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    size_t const cap = 2 * scc_ringdeque_capacity(deque);
    unsigned i;
    for(i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_front(&deque, i));
    }

    while(!scc_ringdeque_empty(deque)) {
        TEST_ASSERT_EQUAL_UINT32(--i, scc_ringdeque_pop_front(deque));
    }

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_back(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    size_t const cap = 2 * scc_ringdeque_capacity(deque);

    for(unsigned  i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_back(&deque, i));
        TEST_ASSERT_EQUAL_UINT32(i, scc_ringdeque_back(deque));
    }

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_front(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    size_t const cap = 2 * scc_ringdeque_capacity(deque);

    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_front(&deque, i));
        TEST_ASSERT_EQUAL_UINT32(i, scc_ringdeque_front(deque));
    }

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_clear(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    size_t const cap = 2 * scc_ringdeque_capacity(deque);

    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_back(&deque, i));
    }

    TEST_ASSERT_EQUAL_UINT64(cap + 0ull, scc_ringdeque_size(deque));

    scc_ringdeque_clear(deque);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_ringdeque_size(deque));

    TEST_ASSERT_TRUE(scc_ringdeque_push_back(&deque, cap + 1u));
    TEST_ASSERT_EQUAL_UINT32(cap + 1u, scc_ringdeque_back(deque));

    scc_ringdeque_clear(deque);

    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_front(&deque, i));
        TEST_ASSERT_EQUAL_UINT32(i, scc_ringdeque_front(deque));
    }

    TEST_ASSERT_EQUAL_UINT64(cap + 0ull, scc_ringdeque_size(deque));
    scc_ringdeque_clear(deque);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_ringdeque_size(deque));

    TEST_ASSERT_TRUE(scc_ringdeque_push_front(&deque, 2 * cap));
    TEST_ASSERT_EQUAL_UINT32(2 * cap, scc_ringdeque_front(deque));

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_reserve_size_is_power_of_2(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);
    TEST_ASSERT_TRUE(scc_ringdeque_reserve(&deque, scc_ringdeque_capacity(deque) + 1u));
    TEST_ASSERT_TRUE(scc_bits_is_power_of_2(scc_ringdeque_capacity(deque)));
    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_reserve_no_reallocation(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);

    size_t const cap = scc_ringdeque_capacity(deque);
    void const *before = deque;
    TEST_ASSERT_TRUE(scc_ringdeque_reserve(&deque, cap - 1u));
    TEST_ASSERT_EQUAL_UINT64(cap, scc_ringdeque_capacity(deque));
    TEST_ASSERT_EQUAL_PTR(before, deque);
    scc_ringdeque_free(deque);
}
