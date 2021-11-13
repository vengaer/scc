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
