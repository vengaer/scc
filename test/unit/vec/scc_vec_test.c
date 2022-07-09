#include <scc/scc_mem.h>
#include <scc/scc_vec.h>

#include <errno.h>
#include <stdio.h>

#include <unity.h>

void test_scc_vec_new(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_impl_base(vec)->sv_size);
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_impl_base(vec)->sv_capacity);
    scc_vec_free(vec);
}

void test_scc_vec_size(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_size(vec));
    scc_vec_free(vec);
}

void test_scc_vec_capacity(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_reserve(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 13));
    TEST_ASSERT_EQUAL_UINT64(13u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 3));
    TEST_ASSERT_EQUAL_UINT64(13u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 32));
    TEST_ASSERT_EQUAL_UINT64(32u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 87));
    TEST_ASSERT_EQUAL_UINT64(87u, (unsigned long long)scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_reserve_zero(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 0u));
    TEST_ASSERT_EQUAL_UINT64(0u, scc_vec_size(vec));
    TEST_ASSERT_EQUAL_UINT64(0u, scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_push(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_push(&vec, 1));
    TEST_ASSERT_EQUAL_UINT64(1u, (unsigned long long)scc_vec_size(vec));
    TEST_ASSERT_EQUAL_INT32(1u, vec[0]);
    TEST_ASSERT_TRUE(scc_vec_push(&vec, 2));
    TEST_ASSERT_EQUAL_UINT64(2u, (unsigned long long)scc_vec_size(vec));
    TEST_ASSERT_EQUAL_INT32(2u, vec[1]);
    scc_vec_free(vec);
}

void test_scc_vec_push_allocation_pattern(void) {
    enum { CHUNKSIZE = 4096 };
    scc_vec(int) vec = scc_vec_new(int);
    size_t ccap = 0u;

    while(ccap < CHUNKSIZE) {
        while(scc_vec_size(vec) < ccap) {
            TEST_ASSERT_TRUE(scc_vec_push(&vec, 1));
            TEST_ASSERT_EQUAL_UINT64(ccap, (unsigned long long)scc_vec_capacity(vec));
        }

        ccap = 2 * ccap + !(ccap & 1);
    }

    while(ccap < 4 * CHUNKSIZE) {
        while(scc_vec_size(vec) < ccap) {
            TEST_ASSERT_TRUE(scc_vec_push(&vec, 1));
            TEST_ASSERT_EQUAL_UINT64(ccap, (unsigned long long)scc_vec_capacity(vec));
        }

        ccap += CHUNKSIZE;
    }


    scc_vec_free(vec);
}

void test_scc_vec_pop(void) {
    enum { TEST_SIZE = 400 };
    scc_vec(int) vec = scc_vec_new(int);

    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, TEST_SIZE));
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE, scc_vec_size(vec));

    for(int i = TEST_SIZE - 1; i >= 0; i--) {
        TEST_ASSERT_EQUAL_INT32(i, vec[scc_vec_size(vec) - 1u]);
        scc_vec_pop(vec);
        TEST_ASSERT_EQUAL_INT32(i, scc_vec_size(vec));
    }

    scc_vec_free(vec);
}

void test_scc_vec_pop_safe(void) {
    enum { TEST_SIZE = 368 };
    scc_vec(int) vec = scc_vec_new(int);

    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, TEST_SIZE));
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE, scc_vec_size(vec));

    for(int i = TEST_SIZE - 1; i >= 0; i--) {
        TEST_ASSERT_EQUAL_INT32(i, vec[scc_vec_size(vec) - 1]);
        scc_vec_pop_safe(vec);
        TEST_ASSERT_EQUAL_INT32(i, scc_vec_size(vec));
    }

    scc_vec_free(vec);
}

void test_scc_vec_at(void) {
    enum { TEST_SIZE = 368 };
    scc_vec(int) vec = scc_vec_new(int);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
        TEST_ASSERT_EQUAL_INT32(i, scc_vec_at(vec, i));
    }

    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 38));
    TEST_ASSERT_EQUAL_UINT64(38u, scc_vec_size(vec));
    for(unsigned i = 0u; i < scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizedown(void) {
    enum { TEST_SIZE = 288 };
    scc_vec(int) vec = scc_vec_new(int);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 10));
    for(int i = 0; i < (int)scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup_sizedown(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 28));
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 12));
    TEST_ASSERT_EQUAL_UINT64(12u, scc_vec_size(vec));
    for(unsigned i = 0u; i < scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup_nonempty(void) {
    enum { TEST_SIZE = 212 };
    scc_vec(int) vec = scc_vec_new(int);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 318));
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    for(unsigned i = TEST_SIZE; i < scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_foreach(void) {
    enum { TEST_SIZE = 2222 };
    scc_vec(int) vec = scc_vec_new(int);

    int i = 0;
    for(; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    i = 0;
    int *iter;
    scc_vec_foreach(iter, vec) {
        TEST_ASSERT_EQUAL_INT32(i++, *iter);
    }

    scc_vec_free(vec);
}

void test_scc_vec_foreach_reversed(void) {
    enum { TEST_SIZE = 2827 };
    scc_vec(int) vec = scc_vec_new(int);

    int i = 0;
    for(; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    int *iter;
    scc_vec_foreach_reversed(iter, vec) {
        TEST_ASSERT_EQUAL_INT32(--i, *iter);
    }
    scc_vec_free(vec);
}

void test_scc_vec_foreach_by(void) {
    enum { TEST_SIZE = 221 };
    enum { STEP = 3 };
    scc_vec(int) vec = scc_vec_new(int);

    int i = 0;
    for(; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    i = 0;
    int *iter;
    scc_vec_foreach_by(iter, vec, STEP) {
        TEST_ASSERT_EQUAL_INT32(i, *iter);
        i += STEP;
    }
    scc_vec_free(vec);
}

void test_scc_vec_foreach_reversed_by(void) {
    enum { TEST_SIZE = 2211 };
    enum { STEP = 8 };
    scc_vec(int) vec = scc_vec_new(int);

    int i = 0;
    for(; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    --i;
    int *iter;
    scc_vec_foreach_reversed_by(iter, vec, STEP) {
        TEST_ASSERT_EQUAL_INT32(i, *iter);
        i -= STEP;
    }
    TEST_ASSERT_LESS_OR_EQUAL(0, i);
    scc_vec_free(vec);
}

void test_scc_vec_empty(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    TEST_ASSERT_TRUE(scc_vec_push(&vec, 1));
    TEST_ASSERT_FALSE(scc_vec_empty(vec));
    scc_vec_pop(vec);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    scc_vec_free(vec);
}

void test_scc_vec_clear(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    TEST_ASSERT_TRUE(scc_vec_push(&vec, 1));
    scc_vec_clear(vec);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    scc_vec_free(vec);
}

void test_scc_vec_erase(void) {
    enum { TEST_SIZE = 212 };
    enum { ERASE_IDX0 = 118 };
    enum { ERASE_IDX1 = 1 };
    scc_vec(int) vec = scc_vec_new(int);

    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec_erase(vec, ERASE_IDX0);
    for(int i = ERASE_IDX0; i < TEST_SIZE - 1; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, vec[i]);
    }

    scc_vec_erase(vec, ERASE_IDX1);
    for(int i = 0; i < ERASE_IDX1; ++i) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    for(int i = ERASE_IDX1; i < ERASE_IDX0 - 1; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, vec[i]);
    }

    for(int i = ERASE_IDX0; i < TEST_SIZE - 2; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 2, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_erase_last(void) {
    enum { TEST_SIZE = 300 };
    scc_vec(int) vec = scc_vec_new(int);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec_erase(vec, TEST_SIZE - 1);
    for(int i = 0; i < TEST_SIZE - 1; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_erase_range(void) {
    enum { TEST_SIZE = 100 };
    enum { ERASE_START = 2 };
    enum { ERASE_END = 8 };
    scc_vec(int) vec = scc_vec_new(int);
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec_erase_range(vec, ERASE_START, ERASE_END);
    for(int i = 0; i < ERASE_START; ++i) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    TEST_ASSERT_EQUAL_INT32(ERASE_END, vec[ERASE_START]);
    int n = ERASE_END - ERASE_START;
    for(int i = 2; i < TEST_SIZE - n; i++) {
        TEST_ASSERT_EQUAL_INT32(i + n, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_erase_range_end_lt_first(void) {
    enum { TEST_SIZE = 100 };
    scc_vec(int) vec = scc_vec_new(int);

    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec_erase_range(vec, 3, 1);
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE, scc_vec_size(vec));
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_erase_range_end_eq_first(void) {
    enum { TEST_SIZE = 100 };
    scc_vec(int) vec = scc_vec_new(int);

    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec_erase_range(vec, 3, 3);
    TEST_ASSERT_EQUAL_UINT64(TEST_SIZE, scc_vec_size(vec));
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_erase_range_end(void) {
    enum { TEST_SIZE = 100 };
    enum { ERASE_START = 50 };
    enum { ERASE_END = 100 };
    scc_vec(int) vec = scc_vec_new(int);

    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }
    int rem = ERASE_END - ERASE_START;

    scc_vec_erase_range(vec, ERASE_START, ERASE_END);
    TEST_ASSERT_EQUAL_UINT64(rem, scc_vec_size(vec));
    for(int i = 0; i < rem; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_from(void) {
    scc_vec(int) vec = scc_vec_from(int, 1, 2, 3, 4, 5);

    TEST_ASSERT_NOT_EQUAL_UINT64(0, (unsigned long long)vec);
    TEST_ASSERT_EQUAL_UINT64(5ull, scc_vec_size(vec));
    TEST_ASSERT_EQUAL_UINT64(5ull, scc_vec_capacity(vec));

    for(int i = 0; i < (int)scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, vec[i]);
    }

    scc_vec_free(vec);
}
