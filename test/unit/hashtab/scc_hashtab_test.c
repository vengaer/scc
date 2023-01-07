#include <inspect/hashtab_inspect.h>
#include <scc/hashtab.h>
#include <scc/mem.h>

#include <stdbool.h>
#include <string.h>

#include <unity.h>

static bool eq(void const *left, void const *right) {
    return *(int const *)left == *(int const *)right;
}

static unsigned long long ident(void const *data, size_t size) {
    (void)size;
    int d = *(int const *)data;
    TEST_ASSERT_GREATER_OR_EQUAL_INT32(0, d);
    return d;
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
    scc_hashtab(int) hashtab = scc_hashtab_new(int, eq);
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
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

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
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
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

/* test_scc_hashtab_reserve
 *
 * Initialize a hash table, reserve a capacity larger than
 * the initial one and verify that the capacity was indeed
 * increased
 */
void test_scc_hashtab_reserve(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    size_t const cap = scc_hashtab_capacity(tab);

    void const *origaddr = tab;
    TEST_ASSERT_TRUE(scc_hashtab_reserve(&tab, 2ul * cap));
    TEST_ASSERT_GREATER_THAN_UINT64(cap, scc_hashtab_capacity(tab));
    TEST_ASSERT_FALSE(origaddr == tab);

    scc_hashtab_free(tab);
}

/* test_scc_hashtab_reserve_retains_values
 *
 * Insert a number of values in the hash table, reserve
 * a capacity large enough that the table has to be rehashed
 * and verify that the values are still present
 */
void test_scc_hashtab_reserve_retains_values(void) {
    enum { TESTSIZE = 256 };
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));

        for(int j = 0; j <= i; ++j) {
            TEST_ASSERT_TRUE(!!scc_hashtab_find(tab, i));
        }
    }
    size_t const cap = scc_hashtab_capacity(tab);
    TEST_ASSERT_TRUE(scc_hashtab_reserve(&tab, (cap << 1u) + 1u));
    TEST_ASSERT_GREATER_THAN_UINT64(cap, scc_hashtab_capacity(tab));

    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_find(tab, i));
    }
    scc_hashtab_free(tab);
}

/* test_scc_hashtab_interleaved_insert_find
 *
 * Perform repeated insertions and, after each insertion,
 * verify that no values are lost
 */
void test_scc_hashtab_interleaved_insert_find(void) {
    enum { TESTSIZE = 4096 };
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    int const *elem;
    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
        for(int j = 0; j <= i; ++j) {
            elem = scc_hashtab_find(tab, j);
            TEST_ASSERT_TRUE(!!elem);
            TEST_ASSERT_EQUAL_INT32(j, *elem);
        }
    }
    scc_hashtab_free(tab);
}

/* test_scc_hashtab_remove
 *
 * Verify that removing values in the table succeeds
 * and that attempting to remove nonexistent ones does
 * not
 */
void test_scc_hashtab_remove(void) {
    enum { VAL = 234 };
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    TEST_ASSERT_FALSE(scc_hashtab_remove(tab, VAL));
    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, VAL));
    TEST_ASSERT_TRUE(scc_hashtab_remove(tab, VAL));
    TEST_ASSERT_FALSE(scc_hashtab_remove(tab, VAL));
    scc_hashtab_free(tab);
}

/* test_scc_hashtab_interleaved_remove_find
 *
 * Insert a large number of elements and remove them
 * one by one. After each removal, verify that removed
 * values are not found by calls to scc_hashtab_find whereas
 * values that have yet to be removed are.
 */
void test_scc_hashtab_interleaved_remove_find(void) {
    enum { TESTSIZE = 2048 };
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
    }

    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, i));

        /* Values not present */
        for(int j = 0; j <= i; ++j) {
            TEST_ASSERT_FALSE(!!scc_hashtab_find(tab, j));
            TEST_ASSERT_FALSE(scc_hashtab_remove(tab, j));
        }

        /* Values still present */
        for(int j = i + 1; j < TESTSIZE; ++j) {
            TEST_ASSERT_TRUE(!!scc_hashtab_find(tab, j));
        }
    }

    scc_hashtab_free(tab);
}

/* test_scc_hashtab_insertion_probe_stop
 *
 * Repeatedly insert and remove values
 * until all slots have been occupied at least
 * once. Insert another value and verify that
 * it does not cause an inifinite loop
 */
void test_scc_hashtab_insertion_probe_stop(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);

    /* Insert and remove until all slots have been
     * used at least once */
    bool done = false;
    for(unsigned i = 0u; !done; ++i) {
        done = true;
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, i));

        for(unsigned j = 0u; j < scc_hashtab_capacity(tab); ++j) {
            if(!md[j]) {
                done = true;
                break;
            }
        }
    }

    /* Should not cause infinite loop*/
    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, 1));

    scc_hashtab_free(tab);
}

/* test_scc_hashtab_find_probe_stop
 *
 * Repeatedly insert and remove values
 * until all slots have been occupied at least
 * once. Run find on the table and verify
 * that it does not cause an infinite loop
 */
void test_scc_hashtab_find_probe_stop(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);

    /* Insert and remove until all slots have been
     * used at least once */
    bool done = false;
    for(unsigned i = 0u; !done; ++i) {
        done = true;
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, i));

        for(unsigned j = 0u; j < scc_hashtab_capacity(tab); ++j) {
            if(!md[j]) {
                done = false;
                break;
            }
        }
    }

    /* Should not cause infinite loop*/
    TEST_ASSERT_FALSE(scc_hashtab_find(tab, 1));

    scc_hashtab_free(tab);
}

void test_scc_hashtab_metadata_mirroring_no_overflow(void) {
    scc_hashtab(int) tab = scc_hashtab_with_hash(int, eq, ident);
    unsigned char const *canary = scc_hashtab_inspect_canary(tab);
    TEST_ASSERT_TRUE(scc_canary_intact(canary, SCC_HASHTAB_CANARYSZ));
    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, SCC_HASHTAB_GUARDSZ));
    TEST_ASSERT_TRUE(scc_canary_intact(canary, SCC_HASHTAB_CANARYSZ));
    scc_hashtab_free(tab);
}
