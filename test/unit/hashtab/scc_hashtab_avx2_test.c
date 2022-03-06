#include <asm/asm_common.h>
#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>

#include <stdbool.h>
#include <string.h>

#include <unity.h>

#ifdef SCC_SIMD_ISA_AVX2

static bool eq(void const *l, void const *r) {
    return *(int const *)l == *(int const *)r;
}

void test_insertion_probe_detects_duplicate(void) {
    enum { TESTVAL = 13 };
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash = scc_hashtab_fnv1a(&(int){ TESTVAL }, sizeof(int));
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    size_t index = hash & (scc_hashtab_capacity(tab) - 1u);
    scc_hashtab_metatype ent = (scc_hashtab_metatype)((hash >> 57) | 0x80);
    int *data = scc_hashtab_inspect_data(tab);

    md[index] = ent;
    if(index < scc_hashtab_impl_guardsz()) {
        md[index + scc_hashtab_capacity(tab)] = ent;
    }

    data[index] = TESTVAL;
    *tab = TESTVAL;

    TEST_ASSERT_EQUAL_UINT64(-1ll, scc_hashtab_probe_insert(base, tab, sizeof(int), hash));
    scc_hashtab_free(tab);
}

void test_insertion_probe_functional_up_to_full_capacity(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
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
        ent = (scc_hashtab_metatype)((hash >> 57) | 0x80);
        md[index] = ent;
        if(index < scc_hashtab_impl_guardsz()) {
            md[index + scc_hashtab_capacity(tab)] = ent;
        }
        data[index] = i;
    }

    for(unsigned i = 0u; i < scc_hashtab_capacity(tab); ++i) {
        TEST_ASSERT_TRUE(md[i] & 0x80);
    }

    scc_hashtab_free(tab);
}

void test_insertion_probe_finds_single_vacant(void) {
    scc_hashtab(int) tab = scc_hashtab_init(int, eq);
    struct scc_hashtab_base *base = scc_hashtab_inspect_base(tab);
    unsigned long long hash = scc_hashtab_fnv1a(tab, sizeof(int));
    size_t slot = hash & (scc_hashtab_capacity(tab) - 1u);
    scc_hashtab_metatype *md = scc_hashtab_inspect_metadata(tab);
    /* Mark all slots as occupied */
    memset(md, 0x80, scc_hashtab_capacity(tab) + scc_hashtab_impl_guardsz());

    size_t insert_slot = slot ? slot - 1u: scc_hashtab_capacity(tab) - 1u;
    /* Clear the very last slot to be probed */
    md[insert_slot] = 0u;
    if(insert_slot < scc_hashtab_impl_guardsz()) {
        md[insert_slot + scc_hashtab_capacity(tab)] = 0u;
    }
    *tab = 32;
    TEST_ASSERT_EQUAL_INT64((long long)insert_slot, scc_hashtab_probe_insert(base, tab, sizeof(int), hash));

    scc_hashtab_free(tab);
}

#endif
