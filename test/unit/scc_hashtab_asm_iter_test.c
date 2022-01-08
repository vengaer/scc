#include <scc/scc_hashtab.h>
#include <scc/scc_mem.h>

#include <unity.h>

#ifndef SCC_SIMD_ISA_UNSUPPORTED

extern void scc_hashtab_impl_prep_iter(scc_hashtab_metatype *md, unsigned long long size);

void test_hashtab_perp_iter_collapse(void) {
    scc_hashtab_metatype md[SCC_VECSIZE];
    static scc_hashtab_metatype const ctrl[] = {
        0x00, 0x8000, 0x4000, 0xc000
    };
    for(unsigned i = 0u; i < SCC_VECSIZE; ++i) {
        md[i] = ctrl[i & (scc_arrsize(ctrl) - 1u)];
    }
    scc_hashtab_impl_prep_iter(md, SCC_VECSIZE);

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

#endif
