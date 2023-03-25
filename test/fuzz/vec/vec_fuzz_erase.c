#include "vec_fuzz_erase.h"

#include <scc/vec.h>

#include <inttypes.h>
#include <stdio.h>

bool vec_fuzz_erase(uint32_t const *data, size_t size) {
    bool success = false;
    scc_vec(uint32_t) vec = scc_vec_new(uint32_t);
    if(!scc_vec_reserve(&vec, size)) {
        fputs("Allocation failure\n", stderr);
        goto epilogue;
    }

    for(unsigned i = 0u; i < size; ++i) {
        scc_vec_push(&vec, data[i]);
    }
    for(unsigned i = 0u; i < size; ++i) {
        scc_vec_erase(vec, 0);
        for(unsigned j = i + 1; j < size; ++j) {
            if(vec[j - i - 1u] != data[j]) {
                fprintf(stderr, "Value %" PRIu32 " lost on single erase\n", data[j]);
                goto epilogue;
            }
        }
    }
    for(unsigned i = 0u; i < size; ++i) {
        scc_vec_push(&vec, data[i]);
    }
    for(unsigned i = 0u; i < size; i += 2) {
        if(scc_vec_size(vec) < 2) {
            break;
        }
        scc_vec_erase_range(vec, 0, 2u);
        for(unsigned j = i + 2; j < size; ++j) {
            if(vec[j - i - 2] != data[j]) {
                fprintf(stderr, "Value %" PRIu32 " lost on range erase\n", data[j]);
                goto epilogue;
            }
        }
        /* For coverage */
        scc_vec_erase_range(vec, 2u, 0);
    }

    success = true;
epilogue:
    scc_vec_free(vec);
    return success;
}
