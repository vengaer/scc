#include <scc/bloom.h>

#include <stdint.h>
#include <stddef.h>

#include <unity.h>

static void murmur_wrapper(struct scc_digest128 *digest, void const *data,
                size_t sz, uint_fast32_t seed) {
    scc_hash_murmur128(digest, data, sz, seed);
}

void test_alternate_allocation(void) {
    scc_bloom(unsigned) flt = scc_bloom_new_dyn(unsigned, 128u, 8u);
    scc_bloom_free(flt);

    flt = scc_bloom_with_hash(unsigned, 128u, 8u, murmur_wrapper);
    scc_bloom_free(flt);

    flt = scc_bloom_with_hash_dyn(unsigned, 128u, 8u, murmur_wrapper);
    scc_bloom_free(flt);
}

void test_simple_insertion_and_lookup(void) {
    scc_bloom(unsigned) flt = scc_bloom_new(unsigned, 128u, 8u);

    for (unsigned i = 0u; i < 512u; ++i) {
        scc_bloom_insert(&flt, i << 1u);
        for (unsigned j = 0u; j < i; ++j)
            TEST_ASSERT_TRUE(scc_bloom_test(flt, i << 1u));
    }

    scc_bloom_free(flt);
}

void test_simple_dynamic_insertion_and_lookup(void) {
    scc_bloom(unsigned) flt = scc_bloom_new_dyn(unsigned, 128u, 8u);

    for (unsigned i = 0u; i < 512u; ++i) {
        scc_bloom_insert(&flt, i << 1u);
        for (unsigned j = 0u; j < i; ++j)
            TEST_ASSERT_TRUE(scc_bloom_test(flt, i << 1u));
    }

    scc_bloom_free(flt);
}

void test_m_lookup(void) {
    scc_bloom(unsigned) flt = scc_bloom_new(unsigned, 5u, 3u);
    TEST_ASSERT_EQUAL_UINT64(1u, scc_bloom_capacity(flt));
    scc_bloom_free(flt);

    flt = scc_bloom_new(unsigned, 11u, 3u);
    TEST_ASSERT_EQUAL_UINT64(2u, scc_bloom_capacity(flt));
    scc_bloom_free(flt);

    flt = scc_bloom_new(unsigned, 170u, 3u);
    TEST_ASSERT_EQUAL_UINT64(22u, scc_bloom_capacity(flt));
    scc_bloom_free(flt);

    flt = scc_bloom_new(unsigned, 1328u, 3u);
    TEST_ASSERT_EQUAL_UINT64(166u, scc_bloom_capacity(flt));
    scc_bloom_free(flt);
}

void test_k_lookup(void) {
    scc_bloom(unsigned) flt = scc_bloom_new(unsigned, 5u, 3u);
    TEST_ASSERT_EQUAL_UINT64(3u, scc_bloom_nhashes(flt));
    scc_bloom_free(flt);

    flt = scc_bloom_new(unsigned, 11u, 8u);
    TEST_ASSERT_EQUAL_UINT64(8u, scc_bloom_nhashes(flt));
    scc_bloom_free(flt);

    flt = scc_bloom_new(unsigned, 170u, 70u);
    TEST_ASSERT_EQUAL_UINT64(70u, scc_bloom_nhashes(flt));
    scc_bloom_free(flt);

    flt = scc_bloom_new(unsigned, 1328u, 88u);
    TEST_ASSERT_EQUAL_UINT64(88u, scc_bloom_nhashes(flt));
    scc_bloom_free(flt);
}
