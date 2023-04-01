#include <inspect/hashtab_inspect.h>

#include <scc/bug.h>
#include <scc/hash.h>
#include <scc/hashtab.h>
#include <scc/mem.h>

#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include <unity.h>

#ifdef SCC_MUTATION_TEST
enum { TEST_SIZE = 64 };
#else
enum { TEST_SIZE = 512 };
#endif

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

void test_scc_hashtab_new_dyn(void) {
    scc_hashtab(int) hashtab = scc_hashtab_new_dyn(int, eq);
    TEST_ASSERT_TRUE(!!hashtab);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(hashtab);
    TEST_ASSERT_TRUE(base->ht_dynalloc);
    scc_hashtab_free(hashtab);
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
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    for(int i = 0; i < TEST_SIZE; ++i) {
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
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));

        for(int j = 0; j <= i; ++j) {
            TEST_ASSERT_TRUE(!!scc_hashtab_find(tab, i));
        }
    }
    size_t const cap = scc_hashtab_capacity(tab);
    TEST_ASSERT_TRUE(scc_hashtab_reserve(&tab, (cap << 1u) + 1u));
    TEST_ASSERT_GREATER_THAN_UINT64(cap, scc_hashtab_capacity(tab));

    for(int i = 0; i < TEST_SIZE; ++i) {
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
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    int const *elem;
    for(int i = 0; i < TEST_SIZE; ++i) {
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
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
    }

    for(int i = 0; i < TEST_SIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_remove(tab, i));

        /* Values not present */
        for(int j = 0; j <= i; ++j) {
            TEST_ASSERT_FALSE(!!scc_hashtab_find(tab, j));
            TEST_ASSERT_FALSE(scc_hashtab_remove(tab, j));
        }

        /* Values still present */
        for(int j = i + 1; j < TEST_SIZE; ++j) {
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

void test_scc_hashtab_rehash_limit(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    size_t cap = scc_hashtab_capacity(tab);
    size_t rehash_lim = (cap * 0.875f) + 1.0;
    for(unsigned i = 0u; i < rehash_lim; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
        TEST_ASSERT_EQUAL_UINT64(cap, scc_hashtab_capacity(tab));
    }

    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, rehash_lim + 1u));
    TEST_ASSERT_GREATER_THAN_UINT64(cap, scc_hashtab_capacity(tab));

    scc_hashtab_free(tab);
}

void test_scc_hashtab_metadata_address_on_rehash(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    size_t cap = scc_hashtab_capacity(tab);
    for(int i = 0; i < (int)cap; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
    }

    TEST_ASSERT_GREATER_THAN_UINT64(cap, scc_hashtab_capacity(tab));
    unsigned char *md = (unsigned char *)tab + (scc_hashtab_capacity(tab) + 1) * sizeof(int);

    /* Align pointer */
    unsigned char lb;
    memcpy(&lb, &md, sizeof(lb));
    size_t const align = scc_alignof(scc_hashtab_metatype);
    lb = (lb + align - 1) & ~(align - 1);
    memcpy(&md, &lb, sizeof(lb));

    TEST_ASSERT_EQUAL_PTR(md, scc_hashtab_inspect_metadata(tab));
    scc_hashtab_free(tab);
}

void test_scc_hashtab_fnv1a64(void) {
    static char const *strings[] = {
        "permian",
        "palaeocene",
        "eocene",
        "oligocene"
    };

    static unsigned long long hashes[] = {
        0x88224508f1e2e011ull,
        0x7b96bcef58fde340ull,
        0xfc0e1183fbd62254ull,
        0x83b3ee57387597f2ull
    };

    scc_static_assert(ULLONG_MAX >= 0xffffffffffffffffull,
                        "Non-confromant implementation (n2310 5.2.4.2.1)");

    unsigned long long hash;
    scc_static_assert(scc_arrsize(strings) == scc_arrsize(hashes));
    for(unsigned i = 0u; i < scc_arrsize(hashes); ++i) {
        hash = scc_hash_fnv1a(strings[i], strlen(strings[i]));
        TEST_ASSERT_EQUAL_UINT64(hashes[i], hash);
    }
}

void test_scc_hashtab_no_unnecessary_reserve_alloc(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    void const *orig = tab;
    TEST_ASSERT_TRUE(scc_hashtab_reserve(&tab, scc_hashtab_capacity(tab)));
    TEST_ASSERT_EQUAL_PTR(orig, tab);

    scc_hashtab_free(tab);
}

void test_scc_hashtab_reverse_non_optimal_capacity(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    TEST_ASSERT_TRUE(scc_hashtab_reserve(&tab, 4155));
    TEST_ASSERT_EQUAL_UINT64(8192ull, scc_hashtab_capacity(tab));

    scc_hashtab_free(tab);
}

void test_scc_hashtab_metadata_scrubbing_on_clear(void) {
    scc_hashtab(int) tab = scc_hashtab_with_hash(int, eq, ident);
    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, (int)scc_hashtab_capacity(tab) - 1));
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_hashtab_size(tab));
    scc_hashtab_clear(tab);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_hashtab_size(tab));

    TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, 1));
    TEST_ASSERT_FALSE(!!scc_hashtab_find(tab, (int)scc_hashtab_capacity(tab) - 1));
    scc_hashtab_free(tab);
}

void test_scc_hashtab_clone(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    for(int i = 0; i < 321; ++i) {
        TEST_ASSERT_TRUE(scc_hashtab_insert(&tab, i));
    }

    scc_hashtab(int) copy = scc_hashtab_clone(tab);

    TEST_ASSERT_EQUAL_UINT64(scc_hashtab_capacity(tab), scc_hashtab_capacity(copy));
    TEST_ASSERT_EQUAL_UINT64(scc_hashtab_size(tab), scc_hashtab_size(copy));

    int const *old;
    int const *new;
    for(int i = 0; i < 321; ++i) {
        old = scc_hashtab_find(tab, i);
        new = scc_hashtab_find(copy, i);
        TEST_ASSERT_TRUE(!!old);
        TEST_ASSERT_TRUE(!!new);
        TEST_ASSERT_EQUAL_INT32(i, *old);
        TEST_ASSERT_EQUAL_INT32(i, *new);
    }

    scc_hashtab_free(tab);
    scc_hashtab_free(copy);
}
