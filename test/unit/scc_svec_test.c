#include <scc/scc_svec.h>

#include <unity.h>

void test_scc_svec_init(void) {
    int *svec = scc_svec_init(int);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_svec_impl_base(svec)->sc_size);
    TEST_ASSERT_EQUAL_UINT64(SCC_SVEC_STATIC_CAPACITY, scc_svec_impl_base(svec)->sc_capacity);
    TEST_ASSERT_EQUAL_UINT8(((unsigned char *)svec)[-1], 0);
    scc_svec_free(svec);
}

void test_scc_svec_size(void) {
    int *svec = scc_svec_init(int);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_svec_size(svec));
    scc_svec_impl_base(svec)->sc_size = 3ull;
    TEST_ASSERT_EQUAL_UINT64(3ull, scc_svec_size(svec));
    scc_svec_impl_base(svec)->sc_size = 18ull;
    TEST_ASSERT_EQUAL_UINT64(18ull, scc_svec_size(svec));
    scc_svec_free(svec);
}

void test_scc_svec_capacity(void) {
    struct foo {
        unsigned long long ull;
        char c;
    };
    struct foo *svec = scc_svec_init(struct foo);
    TEST_ASSERT_EQUAL_UINT64(SCC_SVEC_STATIC_CAPACITY, scc_svec_capacity(svec));
    scc_svec_impl_base(svec)->sc_capacity = 38ull;
    TEST_ASSERT_EQUAL_UINT64(38ull, scc_svec_capacity(svec));
    scc_svec_impl_base(svec)->sc_capacity = 96ull;
    TEST_ASSERT_EQUAL_UINT64(96ull, scc_svec_capacity(svec));
    scc_svec_free(svec);
}

void test_scc_svec_reserve(void) {
    int *svec = scc_svec_init(int);
    TEST_ASSERT_EQUAL_UINT64(SCC_SVEC_STATIC_CAPACITY, scc_svec_capacity(svec));
    TEST_ASSERT_TRUE(scc_svec_reserve(svec, 10));
    TEST_ASSERT_EQUAL_UINT64(SCC_SVEC_STATIC_CAPACITY, scc_svec_capacity(svec));
    TEST_ASSERT_TRUE(scc_svec_reserve(svec, 128));
    TEST_ASSERT_EQUAL_UINT64(128ull, scc_svec_capacity(svec));
    TEST_ASSERT_TRUE(scc_svec_reserve(svec, 18));
    TEST_ASSERT_EQUAL_UINT64(128ull, scc_svec_capacity(svec));
    TEST_ASSERT_TRUE(scc_svec_reserve(svec, 1024));
    TEST_ASSERT_EQUAL_UINT64(1024ull, scc_svec_capacity(svec));
    scc_svec_free(svec);
}

void test_scc_svec_allocation_behavior(void) {
    unsigned *svec = scc_svec_init(unsigned);
    unsigned i = 0u;
    for(i = 0; i < SCC_SVEC_STATIC_CAPACITY; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
        TEST_ASSERT_EQUAL_UINT8(0, ((unsigned char *)svec)[-1]);
    }

    for(unsigned cap = i; i < 4096u; i++) {
        if(i >= cap) {
            cap <<= 1u;
        }
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
        TEST_ASSERT_EQUAL_UINT8(1, ((unsigned char *)svec)[-1]);
        TEST_ASSERT_EQUAL_UINT64(cap, scc_svec_capacity(svec));
    }

    for(unsigned cap = i; i < 16384; i++) {
        if(i >= cap) {
            cap += 4096;
        }
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
        TEST_ASSERT_EQUAL_UINT8(1, ((unsigned char *)svec)[-1]);
        TEST_ASSERT_EQUAL_UINT64(cap, scc_svec_capacity(svec));
    }
    scc_svec_free(svec);
}

void test_scc_svec_push(void) {
    int *svec = scc_svec_init(int);

    for(int i = 0; i < 2 * SCC_SVEC_STATIC_CAPACITY; i++) {
        TEST_ASSERT_EQUAL_UINT64((unsigned long long)i, scc_svec_size(svec));
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
        TEST_ASSERT_EQUAL_INT32(i, svec[i]);
    }
    scc_svec_free(svec);
}

void test_scc_svec_from(void) {
    int *svec = scc_svec_from(int, 1, 2, 3, 4, 5);

    TEST_ASSERT_NOT_EQUAL_UINT64(0, (unsigned long long)svec);
    TEST_ASSERT_EQUAL_UINT64(5ull, scc_svec_size(svec));
    TEST_ASSERT_EQUAL_UINT64(SCC_SVEC_STATIC_CAPACITY, scc_svec_capacity(svec));

    for(int i = 0; i < (int)scc_svec_size(svec); i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, svec[i]);
    }

    scc_svec_free(svec);

    svec = scc_svec_from(int, 1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                              11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                              21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                              31, 32, 33, 34, 35, 36, 37, 38, 39, 40);

    TEST_ASSERT_NOT_EQUAL_UINT64(0, (unsigned long long)svec);
    TEST_ASSERT_EQUAL_UINT64(40, scc_svec_size(svec));
    TEST_ASSERT_EQUAL_UINT64(40, scc_svec_capacity(svec));
    TEST_ASSERT_GREATER_THAN_UINT64(SCC_SVEC_STATIC_CAPACITY, scc_svec_size(svec));
    for(int i = 0; i < (int)scc_svec_size(svec); i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, svec[i]);
    }
    TEST_ASSERT_EQUAL_UINT8(1, ((unsigned char *)svec)[-1]);
    scc_svec_free(svec);
}

void test_scc_svec_empty(void) {
    int *svec = scc_svec_init(int);
    TEST_ASSERT_TRUE(scc_svec_empty(svec));
    TEST_ASSERT_TRUE(scc_svec_push(svec, 1));
    TEST_ASSERT_FALSE(scc_svec_empty(svec));
    scc_svec_free(svec);
}

void test_scc_svec_clear(void) {
    int *svec = scc_svec_from(int, 1, 2, 3);
    TEST_ASSERT_FALSE(scc_svec_empty(svec));
    scc_svec_clear(svec);
    TEST_ASSERT_TRUE(scc_svec_empty(svec));
    scc_svec_free(svec);
}

void test_scc_svec_resize_sizeup(void) {
    int *svec = scc_svec_init(int);
    TEST_ASSERT_TRUE(scc_svec_resize(svec, 38));
    TEST_ASSERT_EQUAL_UINT64(38u, scc_svec_size(svec));
    for(unsigned i = 0u; i < scc_svec_size(svec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, svec[i]);
    }
    scc_svec_free(svec);
}

void test_scc_svec_resize_sizedown(void) {
    int *svec = scc_svec_init(int);
    for(int i = 0; i < 288; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }
    TEST_ASSERT_TRUE(scc_svec_resize(svec, 10));
    for(int i = 0; i < (int)scc_svec_size(svec); i++) {
        TEST_ASSERT_EQUAL_INT32(i, svec[i]);
    }
    scc_svec_free(svec);
}

void test_scc_svec_resize_sizeup_sizedown(void) {
    int *svec = scc_svec_init(int);
    TEST_ASSERT_TRUE(scc_svec_resize(svec, 28));
    TEST_ASSERT_TRUE(scc_svec_resize(svec, 12));
    TEST_ASSERT_EQUAL_UINT64(12u, scc_svec_size(svec));
    for(unsigned i = 0u; i < scc_svec_size(svec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, svec[i]);
    }
    scc_svec_free(svec);
}

void test_scc_svec_resize_sizeup_nonempty(void) {
    int *svec = scc_svec_init(int);
    for(int i = 0; i < 212; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }
    TEST_ASSERT_TRUE(scc_svec_resize(svec, 318));
    for(int i = 0; i < 212; i++) {
        TEST_ASSERT_EQUAL_INT32(i, svec[i]);
    }
    for(unsigned i = 212; i < scc_svec_size(svec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, svec[i]);
    }

    scc_svec_free(svec);
}

void test_scc_svec_pop(void) {
    int *svec = scc_svec_from(int, 1, 2, 3);
    TEST_ASSERT_EQUAL_UINT64(3ull, scc_svec_size(svec));
    scc_svec_pop(svec);
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_svec_size(svec));
    scc_svec_pop(svec);
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_svec_size(svec));
    scc_svec_pop(svec);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_svec_size(svec));
    scc_svec_free(svec);
}

void test_scc_svec_pop_safe(void) {
    int *svec = scc_svec_from(int, 1, 2, 3);
    TEST_ASSERT_EQUAL_UINT64(3ull, scc_svec_size(svec));
    scc_svec_pop_safe(svec);
    TEST_ASSERT_EQUAL_UINT64(2ull, scc_svec_size(svec));
    scc_svec_pop_safe(svec);
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_svec_size(svec));
    scc_svec_pop_safe(svec);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_svec_size(svec));
    scc_svec_free(svec);
}

void test_scc_svec_erase(void) {
    int *svec = scc_svec_init(int);

    for(int i = 0; i < 212; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    scc_svec_erase(svec, &svec[118]);
    for(int i = 118; i < 211; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, svec[i]);
    }

    scc_svec_erase(svec, &svec[1]);
    TEST_ASSERT_EQUAL_INT32(0, svec[0]);
    for(int i = 1; i < 117; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, svec[i]);
    }

    for(int i = 118; i < 210; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 2, svec[i]);
    }

    scc_svec_free(svec);
}

void test_scc_svec_erase_last(void) {
    int *svec = scc_svec_init(int);
    for(int i = 0; i < 300; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    scc_svec_erase(svec, &svec[299]);
    for(int i = 0; i < 299; i++) {
        TEST_ASSERT_EQUAL_INT32(i, svec[i]);
    }
    scc_svec_free(svec);
}

void test_scc_svec_erase_range(void) {
    int *svec = scc_svec_init(int);
    for(int i = 0; i < 100; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    scc_svec_erase_range(svec, &svec[2], &svec[8]);
    TEST_ASSERT_EQUAL_INT32(0, svec[0]);
    TEST_ASSERT_EQUAL_INT32(1, svec[1]);
    TEST_ASSERT_EQUAL_INT32(8, svec[2]);
    for(int i = 2; i < 94; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 6, svec[i]);
    }

    scc_svec_free(svec);
}

void test_scc_svec_erase_range_end_lt_first(void) {
    int *svec = scc_svec_init(int);

    for(int i = 0; i < 100; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    scc_svec_erase_range(svec, &svec[3], &svec[1]);
    TEST_ASSERT_EQUAL_UINT64(100, scc_svec_size(svec));
    for(int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_INT32(i, svec[i]);
    }
    scc_svec_free(svec);
}

void test_scc_svec_erase_range_end_eq_first(void) {
    int *svec = scc_svec_init(int);

    for(int i = 0; i < 100; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    scc_svec_erase_range(svec, &svec[3], &svec[3]);
    TEST_ASSERT_EQUAL_UINT64(100, scc_svec_size(svec));
    for(int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL_INT32(i, svec[i]);
    }
    scc_svec_free(svec);
}

void test_scc_svec_erase_range_end(void) {
    int *svec = scc_svec_init(int);

    for(int i = 0; i < 100; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    scc_svec_erase_range(svec, &svec[50], &svec[100]);
    TEST_ASSERT_EQUAL_UINT64(50, scc_svec_size(svec));
    for(int i = 0; i < 50; i++) {
        TEST_ASSERT_EQUAL_INT32(i, svec[i]);
    }

    scc_svec_free(svec);
}

void test_scc_svec_at(void) {
    int *svec = scc_svec_init(int);
    for(int i = 0; i < 368; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
        TEST_ASSERT_EQUAL_INT32(i, scc_svec_at(svec, i));
    }
    scc_svec_free(svec);
}

void test_scc_svec_foreach(void) {
    int *iter;
    int i = 0;
    int *svec = scc_svec_from(int, 0, 1, 2, 3, 4, 5);
    scc_svec_foreach(iter, svec) {
        TEST_ASSERT_EQUAL_INT32(i++, *iter);
    }

    scc_svec_free(svec);
}

void test_scc_svec_foreach_reversed(void) {
    int *iter;
    int i;
    int *svec = scc_svec_init(int);

    for(i = 0; i < 2827; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    scc_svec_foreach_reversed(iter, svec) {
        TEST_ASSERT_EQUAL_INT32(--i, *iter);
    }
    scc_svec_free(svec);
}

void test_scc_svec_foreach_by(void) {
    int *iter;
    int i;
    int *svec = scc_svec_init(int);

    for(i = 0; i < 221; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    i = 0;
    scc_svec_foreach_by(iter, svec, 3) {
        TEST_ASSERT_EQUAL_INT32(i, *iter);
        i += 3;
    }
    scc_svec_free(svec);
}

void test_scc_svec_foreach_reversed_by(void) {
    int *iter;
    int i;
    int *svec = scc_svec_init(int);

    for(i = 0; i < 2211; i++) {
        TEST_ASSERT_TRUE(scc_svec_push(svec, i));
    }

    --i;
    scc_svec_foreach_reversed_by(iter, svec, 8) {
        TEST_ASSERT_EQUAL_INT32(i, *iter);
        i -= 8;
    }
    TEST_ASSERT_LESS_OR_EQUAL(0, i);
    scc_svec_free(svec);
}
