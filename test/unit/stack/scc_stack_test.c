#include <scc/stack.h>
#include <scc/vec.h>

#include <unity.h>

void test_scc_stack_new(void) {
    scc_stack(int) stack = scc_stack_new(int);
    TEST_ASSERT_EQUAL_UINT64(0, scc_stack_size(stack));
    scc_stack_free(stack);
}

void test_scc_stack_new_dyn(void) {
    scc_stack(int) stack = scc_stack_new_dyn(int);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_stack_size(stack));
    scc_stack_free(stack);
}

void test_scc_stack_push(void) {
    enum { TEST_SIZE = 100 };
    scc_stack(int) stack = scc_stack_new(int);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_stack_push(&stack, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_stack_size(stack));
    }
    scc_stack_free(stack);
}

void test_scc_stack_pop(void) {
    enum { TEST_SIZE = 100 };
    scc_stack(int) stack = scc_stack_new(int);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_stack_push(&stack, i));
    }
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE + 0ull, scc_stack_size(stack));
    for(int i = TEST_SIZE; i > 0; --i) {
        scc_stack_pop(stack);
        TEST_ASSERT_EQUAL_UINT64(i - 1ull, scc_stack_size(stack));
    }
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_stack_size(stack));
    scc_stack_free(stack);
}

void test_scc_stack_top(void) {
    enum { TEST_SIZE = 100 };
    scc_stack(int) stack = scc_stack_new(int);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_stack_push(&stack, i));
        TEST_ASSERT_EQUAL_INT32(i, scc_stack_top(stack));
    }
    for(int i = TEST_SIZE; i > 0; --i) {
        TEST_ASSERT_EQUAL_INT32(i - 1, scc_stack_top(stack));
        scc_stack_pop(stack);
    }
    scc_stack_free(stack);
}

void test_scc_stack_reserve(void) {
    enum { TEST_SIZE = 100 };
    scc_stack(int) stack = scc_stack_new(int);
    TEST_ASSERT_LESS_THAN_UINT64(TEST_SIZE + 0ull, scc_stack_capacity(stack));
    TEST_ASSERT_TRUE(scc_stack_reserve(&stack, TEST_SIZE));
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE + 0ull, scc_stack_capacity(stack));
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_stack_size(stack));
    scc_stack_free(stack);
}

void test_scc_stack_clear(void) {
    enum { TEST_SIZE = 100 };
    scc_stack(int) stack = scc_stack_new(int);
    scc_stack_reserve(&stack, TEST_SIZE);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_stack_push(&stack, i));
    }
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE + 0ull, scc_stack_size(stack));
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE + 0ull, scc_stack_capacity(stack));
    scc_stack_clear(stack);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_stack_size(stack));
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE + 0ull, scc_stack_capacity(stack));
    scc_stack_free(stack);
}

void test_scc_stack_empty(void) {
    scc_stack(int) stack = scc_stack_new(int);

    TEST_ASSERT_TRUE(scc_stack_empty(stack));
    TEST_ASSERT_TRUE(scc_stack_push(&stack, 10));
    TEST_ASSERT_FALSE(scc_stack_empty(stack));
    scc_stack_clear(stack);
    TEST_ASSERT_TRUE(scc_stack_empty(stack));
    scc_stack_free(stack);
}
