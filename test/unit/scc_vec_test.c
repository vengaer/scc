#include <scc/scc_macro.h>
#include <scc/scc_vec.h>

#include <errno.h>
#include <stdio.h>

#include <unity.h>

void test_scc_vec_init(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_container(vec, struct scc_vec, sc_buffer)->sc_size);
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_container(vec, struct scc_vec, sc_buffer)->sc_capacity);
    scc_vec_free(vec);
}

void test_scc_vec_size(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_size(vec));
    scc_vec_free(vec);
}

void test_scc_vec_capacity(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_reserve(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_EQUAL_UINT64(0u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(vec, 13));
    TEST_ASSERT_EQUAL_UINT64(13u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(vec, 3));
    TEST_ASSERT_EQUAL_UINT64(13u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(vec, 32));
    TEST_ASSERT_EQUAL_UINT64(32u, (unsigned long long)scc_vec_capacity(vec));
    TEST_ASSERT_TRUE(scc_vec_reserve(vec, 87));
    TEST_ASSERT_EQUAL_UINT64(87u, (unsigned long long)scc_vec_capacity(vec));
    scc_vec_free(vec);
}

void test_scc_vec_push(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_TRUE(scc_vec_push(vec, 1));
    TEST_ASSERT_EQUAL_UINT64(1u, (unsigned long long)scc_vec_size(vec));
    TEST_ASSERT_EQUAL_INT32(1u, vec[0]);
    TEST_ASSERT_TRUE(scc_vec_push(vec, 2));
    TEST_ASSERT_EQUAL_UINT64(2u, (unsigned long long)scc_vec_size(vec));
    TEST_ASSERT_EQUAL_INT32(2u, vec[1]);
    scc_vec_free(vec);
}

void test_scc_vec_push_allocation_pattern(void) {
    int *vec = scc_vec_init();
    size_t ccap = 0u;

    while(ccap < 4096u) {
        while(scc_vec_size(vec) < ccap) {
            TEST_ASSERT_TRUE(scc_vec_push(vec, 1));
            TEST_ASSERT_EQUAL_UINT64(ccap, (unsigned long long)scc_vec_capacity(vec));
        }

        ccap = 2 * ccap + !(ccap & 1);
    }

    while(ccap < 16384u) {
        while(scc_vec_size(vec) < ccap) {
            TEST_ASSERT_TRUE(scc_vec_push(vec, 1));
            TEST_ASSERT_EQUAL_UINT64(ccap, (unsigned long long)scc_vec_capacity(vec));
        }

        ccap += 4096u;
    }


    scc_vec_free(vec);
}

void test_scc_vec_pop(void) {
    int *vec = scc_vec_init();

    TEST_ASSERT_TRUE(scc_vec_reserve(vec, 400u));
    for(int i = 0; i < 400; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }
    TEST_ASSERT_EQUAL_UINT64(400u, scc_vec_size(vec));

    for(int i = 399; i >= 0; i--) {
        TEST_ASSERT_EQUAL_INT32(i, vec[scc_vec_size(vec) - 1u]);
        scc_vec_pop(vec);
        TEST_ASSERT_EQUAL_INT32(i, scc_vec_size(vec));
    }

    scc_vec_free(vec);
}

void test_scc_vec_pop_safe(void) {
    int *vec = scc_vec_init();

    TEST_ASSERT_TRUE(scc_vec_reserve(vec, 368u));
    for(int i = 0; i < 368; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }
    TEST_ASSERT_EQUAL_UINT64(368u, scc_vec_size(vec));

    for(int i = 367; i >= 0; i--) {
        TEST_ASSERT_EQUAL_INT32(i, vec[scc_vec_size(vec) - 1]);
        scc_vec_pop_safe(vec);
        TEST_ASSERT_EQUAL_INT32(i, scc_vec_size(vec));
    }

    scc_vec_free(vec);
}

void test_scc_vec_at(void) {
    int *vec = scc_vec_init();
    for(int i = 0; i < 368; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
        TEST_ASSERT_EQUAL_INT32(i, scc_vec_at(vec, i));
    }

    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 38));
    TEST_ASSERT_EQUAL_UINT64(38u, scc_vec_size(vec));
    for(unsigned i = 0u; i < scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizedown(void) {
    int *vec = scc_vec_init();
    for(int i = 0; i < 288; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 10));
    for(int i = 0; i < (int)scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup_sizedown(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 28));
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 12));
    TEST_ASSERT_EQUAL_UINT64(12u, scc_vec_size(vec));
    for(unsigned i = 0u; i < scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, vec[i]);
    }
    scc_vec_free(vec);
}

void test_scc_vec_resize_sizeup_nonempty(void) {
    int *vec = scc_vec_init();
    for(int i = 0; i < 212; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }
    TEST_ASSERT_TRUE(scc_vec_resize(vec, 318));
    for(int i = 0; i < 212; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    for(unsigned i = 212; i < scc_vec_size(vec); i++) {
        TEST_ASSERT_EQUAL_INT32(0, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_foreach(void) {
    int *iter;
    int i;
    int *vec = scc_vec_init();

    for(i = 0; i < 2222; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }

    i = 0;
    scc_vec_foreach(iter, vec) {
        TEST_ASSERT_EQUAL_INT32(i++, *iter);
    }

    scc_vec_free(vec);
}

void test_scc_vec_foreach_reversed(void) {
    int *iter;
    int i;
    int *vec = scc_vec_init();

    for(i = 0; i < 2827; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }

    scc_vec_foreach_reversed(iter, vec) {
        TEST_ASSERT_EQUAL_INT32(--i, *iter);
    }
    scc_vec_free(vec);
}

void test_scc_vec_foreach_by(void) {
    int *iter;
    int i;
    int *vec = scc_vec_init();

    for(i = 0; i < 221; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }

    i = 0;
    scc_vec_foreach_by(iter, vec, 3) {
        TEST_ASSERT_EQUAL_INT32(i, *iter);
        i += 3;
    }
    scc_vec_free(vec);
}

void test_scc_vec_foreach_reversed_by(void) {
    int *iter;
    int i;
    int *vec = scc_vec_init();

    for(i = 0; i < 2211; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }

    --i;
    scc_vec_foreach_reversed_by(iter, vec, 8) {
        TEST_ASSERT_EQUAL_INT32(i, *iter);
        i -= 8;
    }
    TEST_ASSERT_LESS_OR_EQUAL(0, i);
    scc_vec_free(vec);
}

void test_scc_vec_empty(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    TEST_ASSERT_TRUE(scc_vec_push(vec, 1));
    TEST_ASSERT_FALSE(scc_vec_empty(vec));
    scc_vec_pop(vec);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    scc_vec_free(vec);
}

void test_scc_vec_clear(void) {
    int *vec = scc_vec_init();
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    TEST_ASSERT_TRUE(scc_vec_push(vec, 1));
    scc_vec_clear(vec);
    TEST_ASSERT_TRUE(scc_vec_empty(vec));
    scc_vec_free(vec);
}

void test_scc_vec_erase(void) {
    int *vec = scc_vec_init();

    for(int i = 0; i < 212; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }

    scc_vec_erase(vec, &vec[118]);
    for(int i = 118; i < 211; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, vec[i]);
    }

    scc_vec_erase(vec, &vec[1]);
    TEST_ASSERT_EQUAL_INT32(0, vec[0]);
    for(int i = 1; i < 117; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 1, vec[i]);
    }

    for(int i = 118; i < 210; i++) {
        TEST_ASSERT_EQUAL_INT32(i + 2, vec[i]);
    }

    scc_vec_free(vec);
}

void test_scc_vec_erase_last(void) {
    int *vec = scc_vec_init();
    for(int i = 0; i < 300; i++) {
        TEST_ASSERT_TRUE(scc_vec_push(vec, i));
    }

    scc_vec_erase(vec, &vec[299]);
    for(int i = 0; i < 299; i++) {
        TEST_ASSERT_EQUAL_INT32(i, vec[i]);
    }
    scc_vec_free(vec);
}

