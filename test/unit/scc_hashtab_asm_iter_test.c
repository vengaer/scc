#include <inspect/scc_hashtab_inspect.h>
#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <unity.h>

#ifndef SCC_SIMD_ISA_UNSUPPORTED

extern void scc_hashtab_impl_prep_iter(scc_hashtab_metatype *md, unsigned long long size);

void test_hashtab_prep_iter_collapse_interleaved(void) {
    scc_hashtab_metatype md[SCC_VECSIZE];
    static scc_hashtab_metatype const ctrl[] = {
        SCC_HASHTAB_INSPECT_VACANT, SCC_HASHTAB_INSPECT_OCCUPIED,
        SCC_HASHTAB_INSPECT_GUARD,  SCC_HASHTAB_INSPECT_VACATED
    };
    for(unsigned i = 0u; i < scc_arrsize(md); ++i) {
        md[i] = ctrl[i & (scc_arrsize(ctrl) - 1u)];
    }
    scc_hashtab_impl_prep_iter(md, scc_arrsize(md));

    for(unsigned i = 0u; i < SCC_VECSIZE; i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(0, md[i]);
    }
    for(unsigned i = 1u; i < SCC_VECSIZE; i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(1, md[i]);
    }
    for(unsigned i = 2u; i < SCC_VECSIZE; i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(0, md[i]);
    }
    for(unsigned i = 3u; i < SCC_VECSIZE; i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(0, md[i]);
    }
}

void test_hashtab_prep_iter_collapse_all_occupied(void) {
    scc_hashtab_metatype md[SCC_VECSIZE];
    for(unsigned i = 0u; i < scc_arrsize(md); ++i) {
        md[i] = SCC_HASHTAB_INSPECT_OCCUPIED;
    }
    scc_hashtab_impl_prep_iter(md, scc_arrsize(md));
    for(unsigned i = 0u; i < scc_arrsize(md); ++i) {
        TEST_ASSERT_EQUAL_UINT16(1, md[i]);
    }
}

void test_hashtab_prep_iter_collapse_guard_hit(void) {
    scc_hashtab_metatype md[SCC_VECSIZE];
    unsigned i = 0u;
    for(; i < scc_arrsize(md) >> 1u; ++i) {
        md[i] = SCC_HASHTAB_INSPECT_OCCUPIED;
    }
    /* The guard can technically never be hit, still,
     * simple enough to test if that were ever to change */
    for(; i < scc_arrsize(md); ++i) {
        md[i] = SCC_HASHTAB_INSPECT_GUARD;
    }
    scc_hashtab_impl_prep_iter(md, scc_arrsize(md));
    for(i = 0u; i < scc_arrsize(md) >> 1u; ++i) {
        TEST_ASSERT_EQUAL_UINT16(1, md[i]);
    }
    for(; i < scc_arrsize(md); ++i) {
        TEST_ASSERT_EQUAL_UINT16(0, md[i]);
    }
}

void test_hashtab_prep_iter_collapse_4x_vector(void) {
    scc_hashtab_metatype md[4 * SCC_VECSIZE];
    static scc_hashtab_metatype const ctrl[] = {
        SCC_HASHTAB_INSPECT_VACANT, SCC_HASHTAB_INSPECT_OCCUPIED,
        SCC_HASHTAB_INSPECT_GUARD,  SCC_HASHTAB_INSPECT_VACATED
    };
    for(unsigned i = 0u; i < scc_arrsize(md); ++i) {
        md[i] = ctrl[(i + 1) & (scc_arrsize(ctrl) - 1u)];
    }
    scc_hashtab_impl_prep_iter(md, scc_arrsize(md));
    for(unsigned i = 0u; i < scc_arrsize(md); i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(1, md[i]);
    }
    for(unsigned i = 1u; i < scc_arrsize(md); i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(0, md[i]);
    }
    for(unsigned i = 2u; i < scc_arrsize(md); i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(0, md[i]);
    }
    for(unsigned i = 3u; i < scc_arrsize(md); i += scc_arrsize(ctrl)) {
        TEST_ASSERT_EQUAL_UINT16(0, md[i]);
    }
}

#endif
