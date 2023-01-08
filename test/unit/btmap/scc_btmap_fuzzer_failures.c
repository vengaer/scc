#include <inspect/btmap_inspect.h>
#include <scc/btmap.h>
#include <scc/mem.h>

#include <stddef.h>
#include <stdint.h>

#include <unity.h>

static int u32compare(void const *l, void const *r) {
    uint32_t leftval = *(uint32_t const *)l;
    uint32_t rightval = *(uint32_t const *)r;
    return (leftval < rightval ? -1 : 1) * !(leftval == rightval);
}

static void run_fuzzer_test(void *restrict map,  uint32_t const *restrict data, size_t ue, size_t size) {
    scc_btmap(uint32_t, uint32_t) btmap = map;

    uint32_t *val;
    for(unsigned i = 0u; i < ue; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, data[i], data[i] << 1u));
        TEST_ASSERT_EQUAL_UINT64(i + 1u, scc_btmap_size(btmap));

        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));
        for(unsigned j = 0u; j < i; ++j) {
            val = scc_btmap_find(btmap, data[j]);
            TEST_ASSERT_TRUE(!!val);
            TEST_ASSERT_EQUAL_UINT32(data[j] << 1u, *val);
        }
    }

    size_t mapsize = scc_btmap_size(btmap);
    for(unsigned i = ue; i < size; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, data[i], 0u));
        TEST_ASSERT_EQUAL_UINT64(mapsize, scc_btmap_size(btmap));

        for(unsigned j = ue; j < i; ++j) {
            val = scc_btmap_find(btmap, data[j]);
            TEST_ASSERT_TRUE(!!val);
            TEST_ASSERT_FALSE(!!*val);
        }
    }

    for(unsigned i = 0u; i < ue; ++i) {
        TEST_ASSERT_TRUE(scc_btmap_insert(&btmap, data[i], data[i] << 1u));
    }

    for(unsigned i = 0u; i < ue; ++i) {
        TEST_ASSERT_EQUAL_UINT64(ue - i, scc_btmap_size(btmap));
        TEST_ASSERT_TRUE(scc_btmap_remove(btmap, data[i]));
        TEST_ASSERT_EQUAL_UINT32(0u, scc_btmap_inspect_invariants(btmap));

        for(unsigned j = i + 1; j < ue; ++j) {
            val = scc_btmap_find(btmap, data[j]);
            TEST_ASSERT_TRUE(!!val);
            TEST_ASSERT_EQUAL_UINT32(data[j] << 1u, *val);
        }
    }

    TEST_ASSERT_EQUAL_UINT64(0ull, scc_btmap_size(btmap));
}

void test_scc_btmap_fuzzer_failure0(void) {
    uint32_t const data[] = {
        0x2b00301bu
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x2bu);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure1(void) {
    uint32_t const data[] = {
        0x92e74848u, 0x48484848u
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x92u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure2(void) {
    uint32_t const data[] = {
        0x03ffffffu, 0xff000000u
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x03u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure3(void) {
    uint32_t const data[] = {
        0xa6ccccccu, 0xccccccccu, 0xccccccccu, 0xccccccccu
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0xa6u);
    run_fuzzer_test(btmap, data, 2u, scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure4(void) {
    uint32_t const data[] = {
        3014917892u, 3014898611u, 565425075u, 3573547007u
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x04u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure5(void) {
    static uint32_t const data[] = {
        0xffffff03u, 0x5dffffff, 0xff00
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x03u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure6(void) {
    static uint32_t const data[] = {
        0x000000003u, 0x5dfffffau, 0x0000ff00u
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x03u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure7(void) {
    static uint32_t const data[] = {
        0xbf001005, 0x00000000, 0xbfff0800, 0x6e747a00,
        0x000d3432, 0x0000003f
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x05u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure8(void) {
    static uint32_t const data[] = {
        0xff001005u, 0x5d0707ffu,  0x0002f1ffu,  0x00000000u,
        0x673a4100u, 0x007fbf6cu,  0x7f00bf00u,  0x000000bfu,
        0x00290000u, 0x2b030000u,  0x5b290000u,  0x7a7eff7fu,
        0xb9346e74u, 0x76727674u,  0x76767676u,  0x36b6ff76u,
        0x7eff7632u, 0x2a3bffffu
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x05u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}

void test_scc_btmap_fuzzer_failure9(void) {
    static uint32_t const data[] = {
        0x12000604u, 0x00000000u, 0x12000201u, 0x00007fffu,
    };

    scc_btmap(uint32_t, uint32_t) btmap = scc_btmap_with_order(uint32_t, uint32_t, u32compare, 0x04u);
    run_fuzzer_test(btmap, data, scc_arrsize(data), scc_arrsize(data));
    scc_btmap_free(btmap);
}
