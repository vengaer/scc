#include "hashtab_fuzz_insertion.h"
#include "hashtab_fuzz_probing.h"
#include "hashtab_fuzz_removal.h"

#include <fuzzer/assertion.h>
#include <fuzzer/dbg.h>
#include <scc/hashtab.h>

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned partition(uint32_t *data, size_t size) {
    bool unique = true;
    unsigned unique_end = 0u;
    /* Stupid partitioning */
    for(unsigned i = 0u; i < size; ++i) {
        unique = true;
        for(unsigned j = 0u; j < i; ++j) {
            if(data[i] == data[j]) {
                unique = false;
                break;
            }
        }
        if(unique) {
            data[unique_end++] = data[i];
        }
    }
    return unique_end;
}

static bool eq(void const *left, void const *right) {
    return *(uint32_t const *)left == *(uint32_t const *)right;
}

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) {
#if defined SCC_FUZZ_SWAR && defined SCC_SIMD_ISA
    extern int scc_simd_support;
    scc_simd_support = 0;
#endif
    size /= sizeof(uint32_t);
    if(!size) {
        return 0;
    }

    uint32_t *buf = malloc(size * sizeof(*buf));
    if(!buf) {
        fputs("Malloc failure\n", stderr);
        abort();
    }
    memcpy(buf, data, size * sizeof(*buf));

    unsigned end = partition(buf, size);

    uint32_t *it;
    dbg_pr("Data:\n");
    dbg_pr_n(it, buf, size, "0x%08" PRIx32 " ", *it);
    dbg_pr("\n");

    scc_hashtab(uint32_t) tab = scc_hashtab_new(uint32_t, eq);

    hashtab_fuzz_probing(&tab, buf, end);
    hashtab_fuzz_insertion(&tab, buf, end, size);
    hashtab_fuzz_removal(&tab, buf, end, size);

    scc_hashtab_free(tab);
    free(buf);
    return 0;
}
