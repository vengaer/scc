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
