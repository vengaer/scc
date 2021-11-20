#include <scc/scc_svec.h>

#include <unity.h>

void test_svec_init(void) {
    int *svec = scc_svec_init(int);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_svec_impl_base(svec)->sc_size);
    TEST_ASSERT_EQUAL_UINT64(SCC_SVEC_STATIC_CAPACITY, scc_svec_impl_base(svec)->sc_capacity);
    TEST_ASSERT_EQUAL_UINT8(((unsigned char *)svec)[-1], 0);
    scc_svec_free(svec);
}
