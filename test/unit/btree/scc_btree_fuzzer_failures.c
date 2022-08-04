#include <inspect/scc_btree_inspect.h>
#include <scc/scc_btree.h>
#include <scc/scc_mem.h>

#include <unity.h>

int compare(void const *l, void const *r) {
    return *(unsigned char const *)l - *(unsigned char const *)r;
}

void test_scc_btree_fuzzer_failure0(void) {
    static unsigned char const data[] = {
        0,   72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  44,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   118, 118, 118, 118, 118

    };
    scc_btree(unsigned char) btree = scc_btree_with_order(unsigned char, compare, 10);

    unsigned char const *p;
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
        for(size_t j = 0u; j < i; ++j) {
            p = scc_btree_find(btree, data[j]);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(*p, data[j]);
        }
    }
    scc_btree_free(btree);
}

void test_scc_btree_fuzzer_failure1(void) {
    static unsigned char const data[] = {
        10,  0,   0,   0,   0,   0,   72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  255, 255,
        255, 179, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 11,  10,  0,   0,   0,   0,   0,   72,  255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 252, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  44,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 0,   0,   0,   0,
        0,   118, 118, 118, 118, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 138, 69,  0,   0,
        144, 97,  0,   0,   255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 72,  72,
        72,  72,  118, 72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  44,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   118, 118,
        118, 118, 118
    };
    scc_btree(unsigned char) btree = scc_btree_with_order(unsigned char, compare, 11);

    unsigned char const *p;
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
        for(size_t j = 0u; j < i; ++j) {
            p = scc_btree_find(btree, data[j]);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(*p, data[j]);
        }
    }
    scc_btree_free(btree);
}

void test_scc_btree_fuzzer_failure2(void) {
    static unsigned char const data[] = {
        0,   72,  72,  72,  72,  72,  72,  72,  0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 1,   0,   0,   0,   0,
        0,   0,   0,   255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 138, 69,  0,   0,   144, 97,
        0,   0,   255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        44,  72,  72,  72,  72,  72,  72,  0, 0, 0, 0, 0, 0,
        0,   0,   72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 1,   0,   0,   0,   0,
        0,   0,   0,   255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 138, 69,  0,   0,   144, 97,
        0,   0,   255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   166, 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        72,  45,  72,  72,  72,  72,  72,  72,  72,  72,  72,
        72,  72,  72,  72,  72,  72,  72,  72,  72,  0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   98,  0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   118, 118, 118, 118, 118
    };
    scc_btree(unsigned char) btree = scc_btree_with_order(unsigned char, compare, 10);

    unsigned char const *p;
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
        for(size_t j = 0u; j < i; ++j) {
            p = scc_btree_find(btree, data[j]);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_INT32(*p, data[j]);
        }
    }
    scc_btree_free(btree);
}

void test_scc_btree_fuzzer_failure3(void) {
    static unsigned char const data[] = { 0x01, 0x00, 0x00, 0x62, 0xb0, 0x00, 0x52, 0xa5, 0x74, 0x00, 0xe9 };
    scc_btree(unsigned char) btree = scc_btree_with_order(unsigned char, compare, 0x04);

    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_remove(btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    scc_btree_free(btree);
}

void test_scc_btree_fuzzer_failure4(void) {
    static unsigned char const data[] = { 0x0e, 0x0e, 0x03, 0x14, 0x00, 0xbb, 0x00, 0x0e, 0x0e, 0x03, 0x14, 0x00, 0xbb, 0x00, 0xe1 };
    scc_btree(unsigned char) btree = scc_btree_with_order(unsigned char, compare, 0x0e);

    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    unsigned char const *p;
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_remove(btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
        for(size_t j = i + 1u; j < scc_arrsize(data); ++j) {
            p = scc_btree_find(btree, data[j]);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_UINT8(data[j], *p);
        }
    }
    scc_btree_free(btree);
}

void test_scc_btree_fuzzer_failure5(void) {
    static unsigned char const data[] = {
        0x01, 0x00, 0x00, 0x62, 0xb0, 0x00, 0x42,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x49, 0x00, 0xe9
    };
    scc_btree(unsigned char) btree = scc_btree_with_order(unsigned char, compare, 0x04);

    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    unsigned char const *p;
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_remove(btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
        for(size_t j = i + 1u; j < scc_arrsize(data); ++j) {
            p = scc_btree_find(btree, data[j]);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_UINT8(data[j], *p);
        }
    }
    scc_btree_free(btree);
}

void test_scc_btree_fuzzer_failure6(void) {
    static unsigned char data[] = {
        0x01, 0x00, 0x00, 0x62, 0xb0, 0x00, 0x42, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa5,
        0x74, 0x00, 0xe9
    };

    scc_btree(unsigned char) btree = scc_btree_with_order(unsigned char, compare, 0x04);

    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_insert(&btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
    }
    unsigned char const *p;
    for(size_t i = 0u; i < scc_arrsize(data); ++i) {
        TEST_ASSERT_TRUE(scc_btree_remove(btree, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btree_inspect_invariants(btree));
        for(size_t j = i + 1u; j < scc_arrsize(data); ++j) {
            p = scc_btree_find(btree, data[j]);
            TEST_ASSERT_TRUE(p);
            TEST_ASSERT_EQUAL_UINT8(data[j], *p);
        }
    }
    scc_btree_free(btree);
}
