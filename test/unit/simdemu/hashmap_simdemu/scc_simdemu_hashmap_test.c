#include <inspect/hashmap_inspect.h>

#include <scc/bug.h>
#include <scc/hashmap.h>
#include <scc/mem.h>

#include <stdbool.h>
#include <stdint.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(uint32_t const *)left == *(uint32_t const *)right;
}

#ifdef SCC_SIMD_ISA
extern int scc_simd_support;
static int simd_backup;

static void disable_simd(void) {
    simd_backup = scc_simd_support;
    scc_simd_support = 0;
}

static void restore_simd(void) {
    scc_simd_support = simd_backup;
}
#else
#define disable_simd() (void)0
#define restore_simd() (void)0
#endif

void test_scc_simdemu_hashmap_kill_insert_mutant(void) {
    disable_simd();
    scc_hashmap(uint32_t, unsigned short) map = scc_hashmap_new(uint32_t, unsigned short, eq);
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

    /* Should not cause infinite loop */
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, 1, 1));

    scc_hashmap_free(map);
    restore_simd();
}
