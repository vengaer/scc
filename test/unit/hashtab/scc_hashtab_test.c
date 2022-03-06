#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <stdbool.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static bool contains(int *tab, int value) {
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    int *data = scc_hashtab_inspect_data(tab);

    for(unsigned i = 0u; i < scc_hashtab_capacity(tab); ++i) {
        if((md[i] & 0x80) && data[i] == value) {
            return true;
        }
    }

    return false;
}

/* test_scc_hashtab_bkoff
 *
 * Verify that the ht_bkoff field has been set correctly
 */
void test_scc_hashtab_bkoff(void) {
    scc_hashtab(int) hashtab = scc_hashtab_init(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(hashtab);
    /* Probably UB... */
    TEST_ASSERT_EQUAL_PTR(base->ht_eq, eq);
    scc_hashtab_free(hashtab);
}

/* test_scc_hashtab_insert
 *
 * Repeatedly insert non-duplicate values and verify that the
 * insertion succeeds
 */
void test_scc_hashtab_insert(void) {
    enum { TESTSIZE = 11 };
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);

    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashtab_size(tab));

        for(int j = 0; j <= i; ++j) {
            TEST_ASSERT_TRUE(contains(tab, j));
        }
    }

    scc_hashtab_free(tab);
}

/* test_scc_hashtab_retained_on_rehash
 *
 * Repeatedly insert values until a rehash is triggered
 * and verify that all values are retained after the
 * rehashing
 */
void test_scc_hashtab_values_retained_on_rehash(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    size_t const cap = scc_hashtab_capacity(tab);

    /* Insert until rehash */
    int i = 0;
    for(; cap == scc_hashtab_capacity(tab); ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
    }

    while(i--) {
        TEST_ASSERT_TRUE(contains(tab, i));
    }

    scc_hashtab_free(tab);
}
