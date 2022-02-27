#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

void test_scc_hashtab_bkoff(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(hashtab);
    /* Probably UB... */
    TEST_ASSERT_EQUAL_PTR(base->ht_eq, eq);
    scc_hashtab_free(hashtab);
}
