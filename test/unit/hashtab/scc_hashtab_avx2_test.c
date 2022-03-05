#include <asm/asm_common.h>
#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>

#include <stdbool.h>

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

#endif
