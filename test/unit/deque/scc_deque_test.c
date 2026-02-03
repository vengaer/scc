#include <scc/deque.h>

#include <unity.h>

void test_scc_deque_new(void) {
    scc_deque(int) deque = scc_deque_new(int);
    TEST_ASSERT_FALSE(((unsigned char *)deque)[-1]);
    scc_deque_free(deque);
}

void test_scc_deque_new_dyn(void) {
    scc_deque(int) deque = scc_deque_new_dyn(int);
    TEST_ASSERT_TRUE(((unsigned char *)deque)[-1]);
    scc_deque_free(deque);
}

void test_scc_deque_push_back(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    size_t cap = scc_deque_capacity(deque);
    for(unsigned i = 0u; i < cap + 3u; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_back(&deque, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_deque_size(deque));

        for(unsigned j = 0u; j < i; ++j) {
            TEST_ASSERT_EQUAL_UINT32(j, deque[j]);
        }
    }
    scc_deque_free(deque);
}

void test_scc_deque_push_front(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    size_t cap = scc_deque_capacity(deque);
    for(unsigned i = 0u; i < 3u * cap; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_front(&deque, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_deque_size(deque));
    }

    struct scc_deque_base *base = scc_deque_impl_base(deque);
    for(unsigned i = base->rd_begin, j = 3u * cap - 1u; i != base->rd_end; i = (i + 1u) & (base->rd_capacity - 1u), --j) {
        TEST_ASSERT_EQUAL_UINT32(j, deque[i]);
    }

    scc_deque_free(deque);
}

void test_scc_deque_pop_back(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    size_t const cap = 2 * scc_deque_capacity(deque);
    unsigned i;
    for(i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_back(&deque, i));
    }

    while(!scc_deque_empty(deque)) {
        TEST_ASSERT_EQUAL_UINT32(--i, scc_deque_pop_back(deque));
    }

    scc_deque_free(deque);
}

void test_scc_deque_pop_front(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    size_t const cap = 2 * scc_deque_capacity(deque);
    unsigned i;
    for(i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_front(&deque, i));
    }

    while(!scc_deque_empty(deque)) {
        TEST_ASSERT_EQUAL_UINT32(--i, scc_deque_pop_front(deque));
    }

    scc_deque_free(deque);
}

void test_scc_deque_back(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    size_t const cap = 2 * scc_deque_capacity(deque);

    for(unsigned  i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_back(&deque, i));
        TEST_ASSERT_EQUAL_UINT32(i, scc_deque_back(deque));
    }

    scc_deque_free(deque);
}

void test_scc_deque_front(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    size_t const cap = 2 * scc_deque_capacity(deque);

    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_front(&deque, i));
        TEST_ASSERT_EQUAL_UINT32(i, scc_deque_front(deque));
    }

    scc_deque_free(deque);
}

void test_scc_deque_clear(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    size_t const cap = 2 * scc_deque_capacity(deque);

    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_back(&deque, i));
    }

    TEST_ASSERT_EQUAL_UINT64(cap + 0ull, scc_deque_size(deque));

    scc_deque_clear(deque);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_deque_size(deque));

    TEST_ASSERT_TRUE(scc_deque_push_back(&deque, cap + 1u));
    TEST_ASSERT_EQUAL_UINT32(cap + 1u, scc_deque_back(deque));

    scc_deque_clear(deque);

    for(unsigned i = 0u; i < cap; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_front(&deque, i));
        TEST_ASSERT_EQUAL_UINT32(i, scc_deque_front(deque));
    }

    TEST_ASSERT_EQUAL_UINT64(cap + 0ull, scc_deque_size(deque));
    scc_deque_clear(deque);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_deque_size(deque));

    TEST_ASSERT_TRUE(scc_deque_push_front(&deque, 2 * cap));
    TEST_ASSERT_EQUAL_UINT32(2 * cap, scc_deque_front(deque));

    scc_deque_free(deque);
}

void test_scc_deque_reserve_size_is_power_of_2(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);
    TEST_ASSERT_TRUE(scc_deque_reserve(&deque, scc_deque_capacity(deque) + 1u));
    TEST_ASSERT_TRUE(scc_bits_is_power_of_2(scc_deque_capacity(deque)));
    scc_deque_free(deque);
}

void test_scc_deque_reserve_no_reallocation(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);

    size_t const cap = scc_deque_capacity(deque);
    void const *before = deque;
    TEST_ASSERT_TRUE(scc_deque_reserve(&deque, cap));
    TEST_ASSERT_EQUAL_UINT64(cap, scc_deque_capacity(deque));
    TEST_ASSERT_EQUAL_PTR(before, deque);
    scc_deque_free(deque);
}

void test_scc_deque_dynalloc_marker(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);

    TEST_ASSERT_TRUE(scc_deque_reserve(&deque, scc_deque_capacity(deque) + 1u));
    TEST_ASSERT_EQUAL_UINT8(1u, ((unsigned char *)deque)[-1]);

    scc_deque_free(deque);
}

void test_scc_deque_power_of_2_rounding(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);

    TEST_ASSERT_TRUE(scc_deque_reserve(&deque, 3084u));
    TEST_ASSERT_EQUAL_UINT64(4096u, scc_deque_capacity(deque));

    scc_deque_free(deque);
}

void test_scc_deque_clone(void) {
    scc_deque(unsigned) deque = scc_deque_new(unsigned);

    for(unsigned i = 0u; i < 320; ++i) {
        TEST_ASSERT_TRUE(scc_deque_push_back(&deque, i));
    }

    scc_deque(unsigned) copy = scc_deque_clone(deque);
    TEST_ASSERT_TRUE(copy);

    TEST_ASSERT_EQUAL_UINT64(320ull, scc_deque_size(deque));

    while(scc_deque_size(deque)) {
        TEST_ASSERT_EQUAL_UINT64(scc_deque_size(deque), scc_deque_size(copy));
        TEST_ASSERT_EQUAL_UINT32(scc_deque_front(deque), scc_deque_front(copy));
        TEST_ASSERT_EQUAL_UINT32(scc_deque_back(deque), scc_deque_back(copy));
        scc_deque_pop_front(deque);
        scc_deque_pop_front(copy);
    }

    scc_deque_free(deque);
    scc_deque_free(copy);
}
