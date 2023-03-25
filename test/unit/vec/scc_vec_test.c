#include <scc/vec.h>

#include <unity.h>

void test_scc_vec_new(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_vec_impl_base(vec)->sv_size);
    TEST_ASSERT_EQUAL_UINT64(SCC_VEC_STATIC_CAPACITY, scc_vec_impl_base(vec)->sv_capacity);
    TEST_ASSERT_EQUAL_UINT8(((unsigned char *)vec)[-1], 0);
    scc_vec_free(vec);
}

void test_scc_vec_new_dyn(void) {
    scc_vec(int) vec = scc_vec_new_dyn(int);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_vec_impl_base(vec)->sv_size);
    TEST_ASSERT_EQUAL_UINT64(SCC_VEC_STATIC_CAPACITY, scc_vec_impl_base(vec)->sv_capacity);
    TEST_ASSERT_EQUAL_UINT8(((unsigned char *)vec)[-1], 1);
    scc_vec_free(vec);
}

void test_scc_vec_size(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_vec_size(vec));
    scc_vec_impl_base(vec)->sv_size = 3ull;  /* NOLINT(readability-magic-numbers) */
    TEST_ASSERT_EQUAL_UINT64(3ull, scc_vec_size(vec));
    scc_vec_impl_base(vec)->sv_size = 18ull; /* NOLINT(readability-magic-numbers) */
    TEST_ASSERT_EQUAL_UINT64(18ull, scc_vec_size(vec));
    scc_vec_free(vec);
}

void test_scc_vec_capacity(void) {
    struct foo {
        unsigned long long ull;
        char c;
    };
    scc_vec(struct foo) vec = scc_vec_new(struct foo);
    TEST_ASSERT_EQUAL_UINT64(SCC_VEC_STATIC_CAPACITY, scc_vec_capacity(vec));
    scc_vec_impl_base(vec)->sv_capacity = 38ull; /* NOLINT(readability-magic-numbers) */
    TEST_ASSERT_EQUAL_UINT64(38ull, scc_vec_capacity(vec));
    scc_vec_impl_base(vec)->sv_capacity = 96ull; /* NOLINT(readability-magic-numbers) */
    TEST_ASSERT_EQUAL_UINT64(96ull, scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_reserve(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_EQUAL_UINT64(SCC_VEC_STATIC_CAPACITY, scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 10));
    TEST_ASSERT_EQUAL_UINT64(SCC_VEC_STATIC_CAPACITY, scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 128));
    TEST_ASSERT_EQUAL_UINT64(128ull, scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 18));
    TEST_ASSERT_EQUAL_UINT64(128ull, scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, 1024));
    TEST_ASSERT_EQUAL_UINT64(1024ull, scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_allocation_behavior(void) {
    enum { CHUNKSIZE = 4096 };
    scc_vec(unsigned) vec = scc_vec_new(unsigned);
    unsigned i = 0u;
    for(i = 0; i < SCC_VEC_STATIC_CAPACITY; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
        TEST_ASSERT_EQUAL_UINT8(0, ((unsigned char *)vec)[-1]);
    }

    for(unsigned cap = i; i < CHUNKSIZE; i++) {
        if(i >= cap) {
            cap = (cap << 1u) | 1u;
        }
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
        TEST_ASSERT_EQUAL_UINT8(1, ((unsigned char *)vec)[-1]);
        TEST_ASSERT_EQUAL_UINT64(cap, scc_vec_capacity(vec));
    }

    for(unsigned cap = scc_vec_capacity(vec); scc_vec_size(vec) < cap; ++i) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, 0));
        TEST_ASSERT_EQUAL_UINT8(1, ((unsigned char *)vec)[-1]);
        TEST_ASSERT_EQUAL_UINT64(cap, scc_vec_capacity(vec));
    }

    for(unsigned cap = i; i < 4 * CHUNKSIZE; i++) {
        if(i >= cap) {
            cap += CHUNKSIZE;
        }
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
        TEST_ASSERT_EQUAL_UINT8(1, ((unsigned char *)vec)[-1]);
        TEST_ASSERT_EQUAL_UINT64(cap, scc_vec_capacity(vec));
    }
    scc_vec_free(vec);
}

void test_scc_vec_push(void) {
    scc_vec(int) vec = scc_vec_new(int);

    for(int i = 0; i < 2 * SCC_VEC_STATIC_CAPACITY; i++) {
        TEST_ASSERT_EQUAL_UINT64((unsigned long long)i, scc_vec_size(vec));
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_from(void) {
    scc_vec(int) vec = scc_vec_from(int, 1, 2, 3, 4, 5);

    TEST_ASSERT_NOT_EQUAL_UINT64(0, (unsigned long long)vec);
    TEST_ASSERT_EQUAL_UINT64(5ull, scc_vec_size(vec));
    TEST_ASSERT_EQUAL_UINT64(SCC_VEC_STATIC_CAPACITY, scc_vec_capacity(vec));

    for(int i = 0; i < (int)scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, vec[i]);
    }

    scc_vec_free(vec);

    vec = scc_vec_from(int, 1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                              11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                              21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                              31, 32, 33, 34, 35, 36, 37, 38, 39, 40);

    TEST_ASSERT_NOT_EQUAL_UINT64(0, (unsigned long long)vec);
    TEST_ASSERT_EQUAL_UINT64(40, scc_vec_size(vec));
    TEST_ASSERT_EQUAL_UINT64(40, scc_vec_capacity(vec));
    TEST_ASSERT_GREATER_THAN_UINT64(SCC_VEC_STATIC_CAPACITY, scc_vec_size(vec));
    for(int i = 0; i < (int)scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, vec[i]);
    }
    TEST_ASSERT_EQUAL_UINT8(1, ((unsigned char *)vec)[-1]);
    scc_vec_free(vec);
}

void test_scc_vec_empty(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    TEST_ASSERT_TRUE(scc_vec_push(&vec, 1));
    TEST_ASSERT_FALSE(scc_vec_empty(vec));
    scc_vec_free(vec);
}

void test_scc_vec_clear(void) {
    scc_vec(int) vec = scc_vec_from(int, 1, 2, 3);
    TEST_ASSERT_FALSE(scc_vec_empty(vec));
    scc_vec_clear(vec);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 38));
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
    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 10));
    for(int i = 0; i < (int)scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup_sizedown(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 28));
    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 12));
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
    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 318));
    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    for(unsigned i = TEST_SIZE; i < scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_pop(void) {
    int *vec = scc_vec_from(int, 1, 2, 3);
    TEST_ASSERT_EQUAL_UINT64(3ull, scc_vec_size(vec));
    scc_vec_pop(vec);
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_vec_size(vec));
    scc_vec_pop(vec);
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_vec_size(vec));
    scc_vec_pop(vec);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_vec_size(vec));
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
    for(int i = 0; i < ERASE_IDX1; i++) {
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
    int rem = TEST_SIZE - (ERASE_END - ERASE_START);

    scc_vec_erase_range(vec, ERASE_START, ERASE_END);
    for(int i = 0; i < ERASE_START; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    TEST_ASSERT_EQUAL_INT32(ERASE_END, vec[ERASE_START]);
    for(int i = ERASE_START; i < rem; i++) {
        TEST_ASSERT_EQUAL_INT32(i + (ERASE_END - ERASE_START), vec[i]);
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
    scc_vec(int) vec = scc_vec_new(int);

    for(int i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec_erase_range(vec, ERASE_START, TEST_SIZE);
    int rem = TEST_SIZE - ERASE_START;

    TEST_ASSERT_EQUAL_UINT64(rem, scc_vec_size(vec));
    for(int i = 0; i < rem; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_foreach(void) {
    int *iter;
    int i = 0;
    int *vec = scc_vec_from(int, 0, 1, 2, 3, 4, 5);
    scc_vec_foreach(iter, vec) {
        TEST_ASSERT_EQUAL_INT32(i++, *iter);
    }

    scc_vec_free(vec);
}

void test_scc_vec_foreach_reversed(void) {
    enum { TEST_SIZE = 2827 };
    int *iter;
    int i;
    scc_vec(int) vec = scc_vec_new(int);

    for(i = 0; i < TEST_SIZE; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec_foreach_reversed(iter, vec) {
        TEST_ASSERT_EQUAL_INT32(--i, *iter);
    }
    scc_vec_free(vec);
}

void test_scc_vec_foreach_by(void) {
    enum { TEST_SIZE = 221 };
    enum { STEP = 3 };
    scc_vec(int) vec = scc_vec_new(int);

    int i;
    for(i = 0; i < TEST_SIZE; i++) {
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
    enum { STEP = 8 };
    enum { TEST_SIZE = 2211 };
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

void test_scc_vec_from_via_malloc(void) {
    scc_vec(int) vec = scc_vec_from(int,
        0,  1,  2,  3,  4,  5,  6,  7,
        8,  9,  10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31,
        32
    );

    TEST_ASSERT_EQUAL_UINT64(33ull, scc_vec_size(vec));

    for(unsigned i = 0u; i < scc_vec_size(vec); ++i) {
        TEST_ASSERT_EQUAL_INT32((int)i, vec[i]);
    }

    /* Should be dynamically allocated */
    TEST_ASSERT_TRUE(scc_vec_is_allocd(vec));

    scc_vec_free(vec);
}

void test_scc_vec_stack_allocation(void) {
    scc_vec(int) vec = scc_vec_from(int,
        0,  1,  2,  3,  4,  5,  6,  7,
        8,  9,  10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31
    );

    /* Should not be dynamically allocated */
    TEST_ASSERT_FALSE(scc_vec_is_allocd(vec));

    scc_vec_free(vec);
}

void test_scc_vec_erase_range_value_retention(void) {
    scc_vec(int) vec = scc_vec_from(int, 0, 1, 2, 3, 4, 5, 6);
    scc_vec_erase_range(vec, 1, 2);
    TEST_ASSERT_EQUAL_INT32(0, vec[0]);
    TEST_ASSERT_EQUAL_INT32(2, vec[1]);
    TEST_ASSERT_EQUAL_INT32(3, vec[2]);
    TEST_ASSERT_EQUAL_INT32(4, vec[3]);
    TEST_ASSERT_EQUAL_INT32(5, vec[4]);
    TEST_ASSERT_EQUAL_INT32(6, vec[5]);

    /* Should not have moved anything beyond the end of the vector */
    TEST_ASSERT_EQUAL_INT32(6, vec[6]);

    scc_vec_free(vec);
}

void test_scc_vec_alloc_on_reverse_only_if_required(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_reserve(&vec, SCC_VEC_STATIC_CAPACITY));
    TEST_ASSERT_FALSE(scc_vec_is_allocd(vec));
    scc_vec_free(vec);
}

void test_scc_vec_resize_no_unnecessary_realloc(void) {
    scc_vec(int) vec = scc_vec_new(int);
    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 31));
    void *va = vec;
    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 31));
    TEST_ASSERT_EQUAL_PTR(va, vec);
    scc_vec_free(vec);
}

void test_scc_vec_sizeup_limit(void) {
    scc_vec(int) vec = scc_vec_new(int);

    TEST_ASSERT_TRUE(scc_vec_resize(&vec, 4096));
    TEST_ASSERT_TRUE(scc_vec_push(&vec, 0));
    /* To catch specific mutation when recalculating capacity */
    TEST_ASSERT_EQUAL_UINT64((4096u << 1u) + 1ull, scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_clone(void) {
    scc_vec(int) vec = scc_vec_new(int);

    for(int i = 0; i < 3200; ++i) {
        TEST_ASSERT_TRUE(scc_vec_push(&vec, i));
    }

    scc_vec(int) clone = scc_vec_clone(vec);
    TEST_ASSERT_TRUE(!!clone);
    TEST_ASSERT_EQUAL_UINT64(scc_vec_size(vec), scc_vec_size(clone));
    TEST_ASSERT_EQUAL_UINT64(scc_vec_capacity(vec), scc_vec_capacity(clone));

    for(unsigned i = 0u; i < scc_vec_capacity(vec); ++i) {
        TEST_ASSERT_EQUAL_INT32(clone[i], vec[i]);
    }

    scc_vec_free(vec);
    scc_vec_free(clone);
}

void test_scc_vec_from_dyn(void) {
    scc_vec(int) vec = scc_vec_from_dyn(int, 0, 1, 2);
    TEST_ASSERT_EQUAL_UINT8(((unsigned char *)vec)[-1], 1);
    scc_vec_free(vec);
}

void test_scc_vec_from_dyn_over_cap(void) {
    TEST_ASSERT_EQUAL_UINT32(32u, SCC_VEC_STATIC_CAPACITY);
    scc_vec(int) vec = scc_vec_from_dyn(int,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        1
    );
    TEST_ASSERT_EQUAL_UINT8(((unsigned char *)vec)[-1], 1);
    TEST_ASSERT_EQUAL_INT32(1, vec[SCC_VEC_STATIC_CAPACITY]);
    scc_vec_free(vec);
}
