#include <scc/scc_macro.h>
#include <scc/scc_vec.h>

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
