#include <scc/ringdeque.h>

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
    for(unsigned i = 0u; i < 3u * cap; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_front(&deque, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_ringdeque_size(deque));
    }

    struct scc_ringdeque_base *base = scc_ringdeque_impl_base(deque);
    for(unsigned i = base->rd_begin, j = 3u * cap - 1u; i != base->rd_end; i = (i + 1u) & (base->rd_capacity - 1u), --j) {
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
    TEST_ASSERT_TRUE(scc_ringdeque_reserve(&deque, cap));
    TEST_ASSERT_EQUAL_UINT64(cap, scc_ringdeque_capacity(deque));
    TEST_ASSERT_EQUAL_PTR(before, deque);
    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_dynalloc_marker(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);

    TEST_ASSERT_TRUE(scc_ringdeque_reserve(&deque, scc_ringdeque_capacity(deque) + 1u));
    TEST_ASSERT_EQUAL_UINT8(1u, ((unsigned char *)deque)[-1]);

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_power_of_2_rounding(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);

    TEST_ASSERT_TRUE(scc_ringdeque_reserve(&deque, 3084u));
    TEST_ASSERT_EQUAL_UINT64(4096u, scc_ringdeque_capacity(deque));

    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_clone(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_new(unsigned);

    for(unsigned i = 0u; i < 320; ++i) {
        TEST_ASSERT_TRUE(scc_ringdeque_push_back(&deque, i));
    }

    scc_ringdeque(unsigned) copy = scc_ringdeque_clone(deque);
    TEST_ASSERT_TRUE(copy);

    TEST_ASSERT_EQUAL_UINT64(320ull, scc_ringdeque_size(deque));

    while(scc_ringdeque_size(deque)) {
        TEST_ASSERT_EQUAL_UINT64(scc_ringdeque_size(deque), scc_ringdeque_size(copy));
        TEST_ASSERT_EQUAL_UINT32(scc_ringdeque_front(deque), scc_ringdeque_front(copy));
        TEST_ASSERT_EQUAL_UINT32(scc_ringdeque_back(deque), scc_ringdeque_back(copy));
        scc_ringdeque_pop_front(deque);
        scc_ringdeque_pop_front(copy);
    }

    scc_ringdeque_free(deque);
    scc_ringdeque_free(copy);
}
