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
