#include <scc/scc_ringdeque.h>

#include <unity.h>

void test_scc_ringdeque_init(void) {
    scc_ringdeque(int) deque = scc_ringdeque_init(int);
    scc_ringdeque_free(deque);
}

void test_scc_ringdeque_push_back(void) {
    scc_ringdeque(unsigned) deque = scc_ringdeque_init(unsigned);
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
    scc_ringdeque(unsigned) deque = scc_ringdeque_init(unsigned);
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
