#include <scc/scc_hashmap.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

void test_scc_hashmap_init(void) {
    scc_hashmap(int, unsigned) map = scc_hashmap_init(int, unsigned, eq);
    scc_hashmap_free(map);
}
