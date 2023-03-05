#include <inspect/hashtab_inspect.h>
#include <scc/arch.h>
#include <scc/hashtab.h>

#include <stdbool.h>
#include <string.h>

#include <unity.h>

#ifdef SCC_SIMD_ISA_AVX2

static bool eq(void const *l, void const *r) {
    return *(int const *)l == *(int const *)r;
}

/* test_insertion_probe_detects_duplicate
 *
 * Compute index to insert value in, manually insert the
 * value and verify that the inserted value is detected
 * while probing.
 */
void test_insertion_probe_detects_duplicate(void) {
    enum { TESTVAL = 13 };
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash = scc_hashtab_fnv1a(&(int){ TESTVAL }, sizeof(int));
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    size_t index = hash & (scc_hashtab_capacity(tab) - 1u);
    scc_hashtab_metatype ent = (scc_hashtab_metatype)((hash >> 57) | 0x80);
    int *data = scc_hashtab_inspect_data(tab);

    md[index] = ent;
    if(index < SCC_HASHTAB_GUARDSZ) {
        md[index + scc_hashtab_capacity(tab)] = ent;
    }

    data[index] = TESTVAL;
    *tab = TESTVAL;

    TEST_ASSERT_EQUAL_UINT64(-1ll, scc_hashtab_probe_insert(base, tab, sizeof(int), hash));
    scc_hashtab_free(tab);
}

/* test_insertion_probe_functional_up_to_full_capacity
 *
 * Manually insert up to scc_hashtab_capacity(tab) elements in
 * the table and verify that the probing detects vacant slots
 */
void test_insertion_probe_functional_up_to_full_capacity(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash;
    size_t index;
    scc_hashtab_metatype ent;
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    int *data = scc_hashtab_inspect_data(tab);
    for(unsigned i = 0u; i < scc_hashtab_capacity(tab); ++i) {
        hash = scc_hashtab_fnv1a(&(int){ i }, sizeof(int));
        *tab = i;
        index = scc_hashtab_probe_insert(base, tab, sizeof(int), hash);
        TEST_ASSERT_NOT_EQUAL_INT64(-1ll, index);
        TEST_ASSERT_FALSE(md[index] & 0x80);
        ent = (scc_hashtab_metatype)((hash >> 57) | 0x80);
        md[index] = ent;
        if(index < SCC_HASHTAB_GUARDSZ) {
            md[index + scc_hashtab_capacity(tab)] = ent;
        }
        data[index] = i;
    }

    for(unsigned i = 0u; i < scc_hashtab_capacity(tab); ++i) {
        TEST_ASSERT_TRUE(md[i] & 0x80);
    }

    scc_hashtab_free(tab);
}

/* test_insertion_probe_finds_single_vacant
 *
 * Fill up the entire hash table apart from the very last slot,
 * probe and verify that the probing finds the vacant slot
 */
void test_insertion_probe_finds_single_vacant(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash = scc_hashtab_fnv1a(tab, sizeof(int));
    size_t slot = hash & (scc_hashtab_capacity(tab) - 1u);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    /* Mark all slots as occupied */
    memset(md, 0x80, scc_hashtab_capacity(tab) + SCC_HASHTAB_GUARDSZ);

    size_t insert_slot;
    /* Clear the very last two slots to be probed */
    for(unsigned i = 0u; i < 2u; ++i) {
        insert_slot = (slot - i - 1u) & (base->ht_capacity - 1u);
        md[insert_slot] = 0u;
        if(insert_slot < SCC_HASHTAB_GUARDSZ) {
            md[insert_slot + base->ht_capacity] = 0u;
        }
    }
    *tab = 32;
    TEST_ASSERT_EQUAL_INT64((long long)insert_slot, scc_hashtab_probe_insert(base, tab, sizeof(int), hash));

    scc_hashtab_free(tab);
}

/* test_insertion_probe_until_stop
 *
 * Compute the index the test value would be inserted
 * and manually mark it as occupied. Manually insert
 * the test value and verify that it is inserted in the
 * proper position. Clear the primary position the value
 * would be inserted in and verify that when probing again,
 * the already inserted value is found.
 */
void test_insertion_probe_until_stop(void) {
    enum { TESTVAL = 328 };

    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);

    *tab = TESTVAL;

    unsigned long long hash = scc_hashtab_fnv1a(tab, sizeof(int));
    size_t slot = hash & (scc_hashtab_capacity(tab) - 1u);

    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    /* Mark primary slot as occupied */
    md[slot] = 0x80;
    if(slot < SCC_HASHTAB_GUARDSZ) {
        md[slot + scc_hashtab_capacity(tab)] = 0x80;
    }

    long long index = scc_hashtab_probe_insert(base, tab, sizeof(int), hash);
    TEST_ASSERT_EQUAL_INT64((slot + 1ll) & (scc_hashtab_capacity(tab) - 1ll), index);

    md[index] = (scc_hashtab_metatype)((hash >> 57) | 0x80);
    if(slot < SCC_HASHTAB_GUARDSZ) {
        md[index + scc_hashtab_capacity(tab)] = (scc_hashtab_metatype)((hash >> 57) | 0x80);
    }

    int *data = scc_hashtab_inspect_data(tab);
    data[index] = TESTVAL;
    /* Probing should detect present value */
    index = scc_hashtab_probe_insert(base, tab, sizeof(int), hash);
    TEST_ASSERT_EQUAL_INT64(-1ll, index);

    /* Vacate primary slot */
    md[slot] = 0x7fu;
    if(slot < SCC_HASHTAB_GUARDSZ) {
        md[slot + scc_hashtab_capacity(tab)] = 0x7fu;
    }

    /* Probing should still detect present value */
    index = scc_hashtab_probe_insert(base, tab, sizeof(int), hash);
    TEST_ASSERT_EQUAL_INT64(-1ll, index);

    scc_hashtab_free(tab);
}

/* test_insertion_probe_no_end_in_vector
 *
 * Find value that would hash to lower vector of table,
 * insert a vector's worth of elements at the would-be
 * index and verify that probing returns the next
 * index beyond the vector
 */
void test_insertion_probe_no_end_in_vector(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);

    /* Reserve enough space to require multiple vector loads */
    size_t const cap = scc_hashtab_capacity(tab) << 2u;
    TEST_ASSERT_GREATER_OR_EQUAL_INT32(SCC_VECSIZE << 1u, cap);
    TEST_ASSERT_TRUE(scc_hashtab_reserve(&tab, cap));
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    int *data = scc_hashtab_inspect_data(tab);

    /* Pick element that hashes to lower half */
    unsigned long long hash;
    size_t index = SIZE_MAX;
    int elem;
    for(elem = 0; index >= SCC_VECSIZE; ++elem) {
        hash = scc_hashtab_fnv1a(&elem, sizeof(elem));
        index = hash & (scc_hashtab_capacity(tab) - 1u);
    }

    scc_hashtab_metatype meta = (scc_hashtab_metatype)(0x80 | (hash >> 57));

    size_t slot;
    /* Set slots to occupied, forcing multiple vector loads */
    for(int i = 0; i < SCC_VECSIZE; ++i) {
        slot = i + index;
        md[slot] = meta;
        if(slot < SCC_HASHTAB_GUARDSZ) {
            md[slot + scc_hashtab_capacity(tab)] = meta;
        }
        /* Ensure eq returns false */
        data[slot] = ~elem;
    }

    TEST_ASSERT_EQUAL_INT64(index + SCC_VECSIZE + 0ll, scc_hashtab_probe_insert(base, tab, sizeof(int), hash));

    scc_hashtab_free(tab);
}

/* test_find_probe_empty
 *
 * Probe empty hash table and verify that the returned index is -1
 */
void test_find_probe_empty(void) {
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    *tab = 32;
    unsigned long long hash = scc_hashtab_fnv1a(tab, sizeof(*tab));
    TEST_ASSERT_EQUAL_INT64(-1ll, scc_hashtab_probe_find(base, tab, sizeof(int), hash));
    scc_hashtab_free(tab);
}

/* test_find_probe_no_match
 *
 * Insert a number of elements in the hash table, probe for one that was not
 * inserted and verify that -1 is returned
 */
void test_find_probe_no_match(void) {
    enum { SIZE = 10 };
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    int *data = scc_hashtab_inspect_data(tab);
    unsigned long long hash;
    size_t index;

    for(int i = 0; i < SIZE; ++i) {
        *tab = i;
        hash = scc_hashtab_fnv1a(tab, sizeof(*tab));
        index = hash & (scc_hashtab_capacity(tab) - 1u);
        md[index] = (scc_hashtab_metatype)((hash >> 57) | 0x80);
        if(index < SCC_HASHTAB_GUARDSZ) {
            md[index + scc_hashtab_capacity(tab)] = (scc_hashtab_metatype)((hash >> 57) | 0x80);
        }
        data[index] = i;
    }
    *tab = SIZE;
    hash = scc_hashtab_fnv1a(tab, sizeof(*tab));
    TEST_ASSERT_EQUAL_INT64(-1ll, scc_hashtab_probe_find(base, tab, sizeof(int), hash));

    scc_hashtab_free(tab);
}

/* test_find_probe_single_value
 *
 * Insert a single value, probe for it and verify that the
 * returned index is correct
 */
void test_find_probe_single_value(void) {
    enum { VAL = 10 };
    scc_hashtab(int) tab = scc_hashtab_new(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    int *data = scc_hashtab_inspect_data(tab);

    *tab = VAL;
    unsigned long long hash = scc_hashtab_fnv1a(tab, sizeof(*tab));
    size_t index = hash & (scc_hashtab_capacity(tab) - 1u);
    md[index] = (scc_hashtab_metatype)((hash >> 57) | 0x80);
    if(index < SCC_HASHTAB_GUARDSZ) {
        md[index + scc_hashtab_capacity(tab)] = (scc_hashtab_metatype)((hash >> 57) | 0x80);
    }
    data[index] = VAL;

    TEST_ASSERT_EQUAL_INT64((long long)index, scc_hashtab_probe_find(base, tab, sizeof(int), hash));

    scc_hashtab_free(tab);
}

#endif
