#include <inspect/hashmap_inspect.h>

#include <scc/hashmap.h>
#include <scc/mem.h>

#include <limits.h>
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

/* test_scc_hashmap_new
 *
 * Initialize a hash map, verify its size and free it
 */
void test_scc_hashmap_new(void) {
    scc_hashmap(int, unsigned) map = scc_hashmap_new(int, unsigned, eq);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_hashmap_size(map));
    scc_hashmap_free(map);
}

/* test_scc_hashmap_insert_changes_size
 *
 * Repeatedly insert elements in the hash map and
 * verify that its size is increased
 */
void test_scc_hashmap_insert_changes_size(void) {
    enum { TESTSIZE = 128 };
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    for(int i = 0u; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, (unsigned short)i));
        TEST_ASSERT_EQUAL_UINT64(i + 1ull, scc_hashmap_size(map));
    }
    scc_hashmap_free(map);
}

/* test_scc_hashmap_find
 *
 * Insert values and verify that every value in
 * the map can be found
 */
void test_scc_hashmap_find(void) {
    enum { TESTSIZE = 512 };
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    unsigned short *val;
    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, (unsigned short)i));
        for(int j = 0; j <= i; ++j) {
            val = scc_hashmap_find(map, j);
            TEST_ASSERT_TRUE(!!val);
        }
    }
    scc_hashmap_free(map);
}

/* test_scc_hashmap_find
 *
 * Insert a value for key 0, find another key that hashes to
 * the same slot and insert a different value. Run find and
 * verify that the returned values were the respectively
 * inserted ones
 */
void test_scc_hashmap_duplicate_insert(void) {
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    struct scc_hashmap_base *base = scc_hashmap_inspect_base(map);

    unsigned long long hash = base->hm_hash(&(int){ 0 }, sizeof(int));
    int dupl = INT_MAX;
    for(int i = 1; i < INT_MAX; ++i) {
        if((base->hm_hash(&(int){ i }, sizeof(int)) & 0x1f) == (hash & 0x1f)) {
            dupl = i;
            break;
        }
    }
    TEST_ASSERT_NOT_EQUAL_INT32(INT_MAX, dupl);

    unsigned short *val;
    /* Insert */
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, 0, 38));
    val = scc_hashmap_find(map, 0);
    TEST_ASSERT_TRUE(!!val);
    TEST_ASSERT_EQUAL_UINT16(38, *val);

    /* Insert for key that hashes to same slot */
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, dupl, 22));

    /* Check that first insertion remains valid */
    val = scc_hashmap_find(map, 0);
    TEST_ASSERT_TRUE(!!val);
    TEST_ASSERT_EQUAL_UINT16(38, *val);

    /* Find for duplicate */
    val = scc_hashmap_find(map, dupl);
    TEST_ASSERT_TRUE(!!val);
    TEST_ASSERT_EQUAL_UINT16(22, *val);

    scc_hashmap_free(map);
}

/* test_scc_hashmap_insert_overrides_existing
 *
 * Insert a value for key 0 and verify that it
 * is found. Insert another values for key 0 and
 * verify that it overwrite the original one
 */
void test_scc_hashmap_insert_overrides_exisiting(void) {
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, 0, 123));
    unsigned short *val = scc_hashmap_find(map, 0);
    TEST_ASSERT_TRUE(!!val);
    TEST_ASSERT_EQUAL_UINT16(123, *val);

    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, 0, 321));
    val = scc_hashmap_find(map, 0);
    TEST_ASSERT_TRUE(!!val);
    TEST_ASSERT_EQUAL_UINT16(321, *val);

    scc_hashmap_free(map);
}

/* test_scc_hashmap_values_retained_across_rehash
 *
 * Repeatedly insert values until a rehash is triggered.
 * Ensure all inserted values are found and correspond to
 * the correct value
 */
void test_scc_hashmap_values_retained_across_rehash(void) {
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    size_t orig_cap = scc_hashmap_capacity(map);

    int i;
    for(i = 0; scc_hashmap_capacity(map) == orig_cap; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, i));
    }

    unsigned short *val;
    for(int j = 0; j < i; ++j) {
        val = scc_hashmap_find(map, j);
        TEST_ASSERT_TRUE(!!val);
        TEST_ASSERT_EQUAL_UINT16((unsigned short)j, *val);
    }

    scc_hashmap_free(map);
}

/* test_scc_hashmap_elements_erased_on_remove
 *
 * Insert a number of values, erase each in order
 * while verifying that the remaining values remain
 * in the map
 */
void test_scc_hashmap_elements_erased_on_remove(void) {
    enum { TESTSIZE = 312 };
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, i));
    }

    unsigned short *val;
    for(int i = 0; i < TESTSIZE; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_remove(map, i));
        TEST_ASSERT_EQUAL_UINT64(TESTSIZE - i - 1ull, scc_hashmap_size(map));

        for(int j = i + 1; j < TESTSIZE; ++j) {
            val = scc_hashmap_find(map, j);
            TEST_ASSERT_TRUE(!!val);
            TEST_ASSERT_EQUAL_UINT16((unsigned short)j, *val);
        }
    }

    scc_hashmap_free(map);
}

/* test_scc_hashmap_insertion_probe_stop
 *
 * Repeatedly insert and remove values
 * until all slots have been occupied at least
 * once. Insert another value and verify that
 * it does not cause an inifinite loop
 */
void test_scc_hashmap_insertion_probe_stop(void) {
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    scc_hashmap_metatype *md = scc_hashmap_inspect_metadata(map);

    /* Insert and remove until all slots have been
     * used at least once */
    bool done = false;
    for(unsigned i = 0u; !done; ++i) {
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
}

/* test_scc_hashmap_find_probe_stop
 *
 * Repeatedly insert and remove values
 * until all slots have been occupied at least
 * once. Run find on the map and verify
 * that it does not cause an infinite loop
 */
void test_scc_hashmap_find_probe_stop(void) {
    scc_hashmap(int, unsigned short) map = scc_hashmap_new(int, unsigned short, eq);
    scc_hashmap_metatype *md = scc_hashmap_inspect_metadata(map);

    /* Insert and remove until all slots have been
     * used at least once */
    bool done = false;
    for(unsigned i = 0u; !done; ++i) {
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
    TEST_ASSERT_FALSE(scc_hashmap_find(map, 1));

    scc_hashmap_free(map);
}

void test_scc_hashmap_rehash_limit(void) {
    scc_hashmap(int, int) map = scc_hashmap_new(int, int, eq);
    size_t cap = scc_hashmap_capacity(map);
    size_t rehash_lim = (cap * 0.875f) + 1.0;
    for(int i = 0u; i < (int)rehash_lim; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, i));
        TEST_ASSERT_EQUAL_UINT64(cap, scc_hashmap_capacity(map));
    }

    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, (int)rehash_lim + 1, 0));
    TEST_ASSERT_GREATER_THAN_UINT64(cap, scc_hashmap_capacity(map));

    scc_hashmap_free(map);
}

void test_scc_hashmap_fnv1a64(void) {
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
        hash = scc_hashmap_fnv1a(strings[i], strlen(strings[i]));
        TEST_ASSERT_EQUAL_UINT64(hashes[i], hash);
    }
}

void test_scc_hashmap_metadata_scrubbing_on_clear(void) {
    scc_hashmap(int, int) map = scc_hashmap_with_hash(int, int, eq, ident);
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, scc_hashmap_capacity(map) - 1, 0));
    TEST_ASSERT_EQUAL_UINT64(1ull, scc_hashmap_size(map));
    scc_hashmap_clear(map);
    TEST_ASSERT_EQUAL_UINT64(0ull, scc_hashmap_size(map));

    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, 1, 0));
    TEST_ASSERT_FALSE(!!scc_hashmap_find(map, (int)scc_hashmap_capacity(map) - 1));
    scc_hashmap_free(map);
}

void test_scc_hashmap_metadata_mirroring_no_overflow(void) {
    scc_hashmap(int, int) map = scc_hashmap_with_hash(int, int, eq, ident);
    unsigned char const *canary = scc_hashmap_inspect_canary(map);
    TEST_ASSERT_TRUE(scc_canary_intact(canary, SCC_HASHMAP_CANARYSZ));
    TEST_ASSERT_TRUE(scc_hashmap_insert(&map, SCC_HASHMAP_GUARDSZ, 0));
    TEST_ASSERT_TRUE(scc_canary_intact(canary, SCC_HASHMAP_CANARYSZ));
    scc_hashmap_free(map);
}

void test_scc_hashmap_clone(void) {
    scc_hashmap(int, int) map = scc_hashmap_new(int, int, eq);
    for(int i = 0; i < 321; ++i) {
        TEST_ASSERT_TRUE(scc_hashmap_insert(&map, i, i + 1));
    }

    scc_hashmap(int, int) copy = scc_hashmap_clone(map);

    TEST_ASSERT_EQUAL_UINT64(scc_hashmap_capacity(map), scc_hashmap_capacity(copy));
    TEST_ASSERT_EQUAL_UINT64(scc_hashmap_size(map), scc_hashmap_size(copy));

    int *old;
    int *new;
    for(int i = 0; i < 321; ++i) {
        old = scc_hashmap_find(map, i);
        new = scc_hashmap_find(copy, i);
        TEST_ASSERT_TRUE(!!old);
        TEST_ASSERT_TRUE(!!new);
        TEST_ASSERT_EQUAL_INT32(i + 1, *old);
        TEST_ASSERT_EQUAL_INT32(i + 1, *new);
    }

    scc_hashmap_free(map);
    scc_hashmap_free(copy);
}
