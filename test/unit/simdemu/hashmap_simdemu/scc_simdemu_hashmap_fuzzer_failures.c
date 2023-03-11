#include <inspect/hashmap_inspect.h>

#include <scc/bug.h>
#include <scc/hashmap.h>
#include <scc/mem.h>

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include <unity.h>

static bool ueq(void const *left, void const *right) {
    return *(uint32_t const *)left == *(uint32_t const *)right;
}

extern int scc_simd_support;
static int simd_backup;

static void disable_simd(void) {
    simd_backup = scc_simd_support;
    scc_simd_support = 0;
}

static void restore_simd(void) {
    scc_simd_support = simd_backup;
}

static void run_fuzzer_test(uint32_t const *restrict keys, uint16_t const *restrict vals, size_t n, size_t totsize) {
    disable_simd();
    scc_hashmap(uint32_t, uint16_t) map = scc_hashmap_new(uint32_t, uint16_t, ueq);
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

    uint16_t val;
    uint16_t *vp;
    uint16_t *vpp;
    size_t cap;
    for(unsigned i = n; i < totsize; ++i) {
        vp = scc_hashmap_find(map, keys[i]);
        cap = scc_hashmap_capacity(map);
        TEST_ASSERT_TRUE(!!vp);
        val = *vp;
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], vals[i]));
        TEST_ASSERT_EQUAL_UINT64(n, scc_hashmap_size(map));
        vpp = scc_hashmap_find(map, keys[i]);
        TEST_ASSERT_TRUE(!!vpp);
        TEST_ASSERT_EQUAL_UINT16(vals[i], *vpp);
        TEST_ASSERT_TRUE(cap != scc_hashmap_capacity(map) || vp == vpp);
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, keys[i], val));
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
    restore_simd();
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
    run_fuzzer_test(keys, vals, scc_arrsize(keys), scc_arrsize(keys));
}

void test_scc_simdemu_hashmap_fuzzer_mutkill0(void) {
    uint32_t keys[] = {
        0x00ff7c87, 0xff250000, 0xc7c7f006, 0x45b1c7c7,
        0x03402d45, 0x007a0000, 0x00000000, 0x6b00002f,
        0x2f2f2f2f, 0x00002f2f, 0xadadad00, 0xadadadad,
        0xadadad2e, 0x0000adad, 0x7cadadad, 0x000000ff,
        0xf006ff25, 0xc7c7c7c7, 0x2d4545b1, 0x00000340,
        0x0000007a, 0x002f0000, 0x2f2f6b00, 0xad000000,
        0xad2eadad, 0xadad0000, 0x2eadadad, 0x2dadadad,
        0xad27adad, 0x000000ad
    };

    uint16_t vals[] = {
        0x7c87, 0x00ff, 0x0000, 0xff25,
        0xf006, 0xc7c7, 0xc7c7, 0x45b1,
        0x2d45, 0x0340, 0x0000, 0x007a,
        0x0000, 0x0000, 0x002f, 0x6b00,
        0x2f2f, 0x2f2f, 0x2f2f, 0x0000,
        0xad00, 0xadad, 0xadad, 0xadad,
        0xad2e, 0xadad, 0xadad, 0x0000,
        0xadad, 0x7cad
    };

    scc_static_assert(scc_arrsize(keys) == scc_arrsize(vals));
    run_fuzzer_test(keys, vals, scc_arrsize(keys), scc_arrsize(keys));
}


void test_scc_simdemu_hashmap_fuzzer_mutkill1(void) {
    uint32_t keys[] = {
        0xa3a3a3a3, 0xe8e80053, 0xe8e8e8e8, 0x000000e8,
        0x01d2f75c, 0x5d7c3d2b, 0x0000003f, 0x19ffffff,
        0x00080100, 0x4c6f0000, 0xffffff7c, 0x012da3a3,
        0x9898982b, 0x3d6f7c7c, 0x004c5d3f, 0x00000000,
        0xfe0a0000, 0x002b5e3d, 0x08fc0000, 0x6f000000,
        0xffff7c4c, 0xa3a32eff, 0x982b012d, 0x98989898,
        0x7c7c9898, 0x5d3f3d6f, 0x0000004c, 0x5e3d0200,
        0x0000002b, 0x02000200, 0xff2d9ba8, 0xa3a36cff,
        0x0000d1a8
    };

    uint16_t vals[] = {
        0xa3a3, 0xa3a3, 0x0053, 0xe8e8,
        0xe8e8, 0xe8e8, 0x00e8, 0x0000,
        0xf75c, 0x01d2, 0x3d2b, 0x5d7c,
        0x003f, 0x0000, 0x0000, 0x0000,
        0xffff, 0x19ff, 0x0000, 0x0000,
        0x0100, 0x0008, 0x0000, 0x4c6f,
        0xff7c, 0xffff, 0xa3a3, 0x012d,
        0x982b, 0x9898, 0x9898, 0x9898,
        0x7c7c
    };

    scc_static_assert(scc_arrsize(keys) == scc_arrsize(vals));
    run_fuzzer_test(keys, vals, scc_arrsize(keys), scc_arrsize(keys));
}

void test_scc_simdemu_hashmap_fuzzer_mutkill2(void) {
    uint32_t keys[] = {
        0x00400000, 0x00000000
    };

    uint16_t vals[] = {
        0x0040, 0x0000
    };

    scc_static_assert(scc_arrsize(keys) == scc_arrsize(vals));
    run_fuzzer_test(keys, vals, scc_arrsize(keys), scc_arrsize(keys));
}

void test_scc_simdemu_hashmap_fuzzer_mutkill3(void) {
    uint32_t keys[] = {
        0xa3a3a3a3, 0xffffffa3, 0x00a3ff, 0x00000000,
        0xc6f93f00, 0x6020000b, 0x6f0000, 0x1f650000,
        0xa8020000, 0xa8a8a89b, 0xa8a8a8, 0xa8000000,
        0xa8000000
    };

    uint16_t vals[] = {
        0xa3a3, 0xa3a3, 0xffa3, 0xffff,
        0xa3ff, 0x0000, 0x0000, 0x0000,
        0x3f00, 0xc6f9, 0x000b, 0x6020,
        0x0000
    };

    scc_static_assert(scc_arrsize(keys) == scc_arrsize(vals));
    run_fuzzer_test(keys, vals, scc_arrsize(keys) - 1u, scc_arrsize(keys));
}

void test_scc_simdemu_hashmap_fuzzer_mutkill4(void) {
    uint32_t keys[] = {
        0x8202d100, 0x8202d100
    };

    uint16_t vals[] = {
        0xd100, 0x8202
    };

    scc_static_assert(scc_arrsize(keys) == scc_arrsize(vals));
    run_fuzzer_test(keys, vals, scc_arrsize(keys) - 1u, scc_arrsize(keys));
}

void test_scc_simdemu_hashmap_fuzzer_mutkill5(void) {
    uint32_t keys[] = {
        0xff7c87,   0x49250000, 0x00000000, 0x45000000,
        0x3402d45,  0x7a0000,   0x6b00002f, 0x2f2f2f2f,
        0x002f2f,   0xadadad00, 0xadadadad, 0xadadad2e,
        0x00adad,   0x7cadadad, 0x0000ff,   0xf006ff25,
        0xc7c7c7c7, 0x2d4545b1, 0x000340,   0x00007a,
        0x2f0000,   0x2f2f6b00, 0xad000000, 0xad2eadad,
        0xadad0000, 0x2eadadad, 0x2dadadad, 0xad27adad,
        0xadad0000, 0xadadadad, 0x2eadadad, 0x2dadadad,
        0xad27adad
    };
    uint16_t vals[] = {
        0x7c87, 0x00ff, 0x0000, 0x4925,
        0x0000, 0x0000, 0x0000, 0x4500,
        0x2d45, 0x340,  0x0000, 0x7a,
        0x0000, 0x0000, 0x2f,   0x6b00,
        0x2f2f, 0x2f2f, 0x2f2f, 0x0000,
        0xad00, 0xadad, 0xadad, 0xadad,
        0xad2e, 0xadad, 0xadad, 0x0000,
        0xadad, 0x7cad, 0x00ff, 0x0000,
        0xff25
    };

    scc_static_assert(scc_arrsize(keys) == scc_arrsize(vals));
    run_fuzzer_test(keys, vals, scc_arrsize(keys) - 5u, scc_arrsize(keys));
}

void test_scc_simdemu_hashmap_fuzzer_mutkill6(void) {
    disable_simd();
    scc_hashmap(uint32_t, unsigned short) map = scc_hashmap_new(uint32_t, unsigned short, ueq);
    scc_hashmap_metatype *md = scc_hashmap_inspect_metadata(map);

    /* Insert and remove until all slots have been
     * used at least once */
    bool done = false;
    for(uint32_t i = 0u; !done; ++i) {
        done = true;
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, i));
        TEST_ASSERT_TRUE(scc_hashmap_remove(map, i));

        for(unsigned j = 0u; j < scc_hashmap_capacity(map); ++j) {
            if(!md[j]) {
                done = false;
                break;
            }
        }
    }

    /* Should not cause infinite loop*/
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, 1, 1));

    scc_hashmap_free(map);
    restore_simd();
}
