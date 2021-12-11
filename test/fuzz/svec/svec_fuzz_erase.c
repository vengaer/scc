#include "svec_fuzz_erase.h"

#include <scc/scc_svec.h>

#include <inttypes.h>
#include <stdio.h>

bool svec_fuzz_erase(uint32_t const *data, size_t size) {
    bool success = false;
    scc_svec(uint32_t) svec = scc_svec_init(uint32_t);
    if(!scc_svec_reserve(svec, size)) {
        fputs("Allocation failure\n", stderr);
        goto epilogue;
    }

    for(unsigned i = 0u; i < size; ++i) {
        scc_svec_push(svec, data[i]);
    }
    for(unsigned i = 0u; i < size; ++i) {
        scc_svec_erase(svec, 0);
        for(unsigned j = i + 1; j < size; ++j) {
            if(svec[j - i - 1u] != data[j]) {
                fprintf(stderr, "Value %" PRIu32 " lost on single erase\n", data[j]);
                goto epilogue;
            }
        }
    }
    for(unsigned i = 0u; i < size; ++i) {
        scc_svec_push(svec, data[i]);
    }
    for(unsigned i = 0u; i < size; i += 2) {
        if(scc_svec_size(svec) < 2) {
            break;
        }
        scc_svec_erase_range(svec, 0, 2u);
        for(unsigned j = i + 2; j < size; ++j) {
            if(svec[j - i - 2] != data[j]) {
                fprintf(stderr, "Value %" PRIu32 " lost on range erase\n", data[j]);
                goto epilogue;
            }
        }
        /* For coverage */
        scc_svec_erase_range(svec, 2u, 0);
    }

    success = true;
epilogue:
    scc_svec_free(svec);
    return success;
}
