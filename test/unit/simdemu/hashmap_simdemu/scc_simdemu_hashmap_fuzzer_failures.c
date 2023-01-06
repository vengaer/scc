#include <inspect/hashmap_inspect.h>

#include <scc/hashmap.h>
#include <scc/mem.h>

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static void run_fuzzer_test(uint32_t const *restrict keys, uint16_t const *restrict vals, size_t n) {
    scc_hashmap(uint32_t, uint16_t) map = scc_hashmap_new(uint32_t, uint16_t, eq);
    uint16_t *elem;
    for(unsigned i = 0u; i < n; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], vals[i]));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashmap_size(map));

        for(unsigned j = 0u; j <= i; ++j) {
            elem = scc_hashmap_find(map, keys[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT16(vals[j], *elem);
        }
    }

    for(unsigned i = 0u; i < n; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_remove(map, keys[i]));
        TEST_ASSERT_EQUAL_UINT64(n - i - 1u, scc_hashmap_size(map));

        for(unsigned j = i + 1u; j < n; ++j) {
            elem = scc_hashmap_find(map, keys[j]);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_UINT16(vals[j], *elem);
        }
    }
    scc_hashmap_free(map);
}

void test_scc_simdemu_hashmap_fuzzer_failure0(void) {
    static uint32_t const keys[] = {
        0x00000000, 0xa3a3a3a3, 0xe8e80053, 0xe8e8e8e8,
        0x000000e8, 0x01d2f75c, 0x5d7c3d2b, 0x0000003f,
        0xe05fffff, 0xc40d0200, 0x19ffffff, 0x80080100,
        0x4c6f0000, 0xffffff7c, 0x012da3a3, 0x9898982b,
        0x98989898, 0x3d6f7c7c, 0x004c5d3f, 0x5e3dfe0a,
        0x0000002b, 0x00000800, 0x7c4c6f00, 0xa3ffffff,
        0x2b012da3, 0x7c989898, 0x3f3d6f7c, 0x00004c5d,
        0xbfbfbf00, 0xbfbf0200, 0x5e3dbfbf
    };

    static uint16_t const vals[] = {
        0x0000, 0x0000, 0x0000, 0x0000,
        0xa3a3, 0xa3a3, 0x0053, 0xe8e8,
        0xe8e8, 0xe8e8, 0x00e8, 0x0000,
        0xf75c, 0x01d2, 0x3d2b, 0x5d7c,
        0x003f, 0x0000, 0x0000, 0x0000,
        0xffff, 0xe05f, 0x0200, 0xc40d,
        0xffff, 0x19ff, 0x0000, 0x0000,
        0x0100, 0x8008, 0x0000
    };

    scc_static_assert(scc_arrsize(keys) == scc_arrsize(vals));
    run_fuzzer_test(keys, vals, scc_arrsize(keys));
}
