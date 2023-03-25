#include "vec_fuzz_push_pop.h"

#include <scc/vec.h>

#include <inttypes.h>
#include <stdio.h>

static bool vec_fuzz_push_pop_test(scc_vec(uint32_t) *vec, uint32_t const *data, size_t size, size_t initsize) {
    for(unsigned i = 0; i < size; ++i) {
        if(!scc_vec_push(vec, data[i])) {
            fprintf(stderr, "Insertion error at element %u\n", i);
            return false;
        }
        if(scc_vec_size(*vec) != i + 1ull + initsize) {
            fprintf(stderr, "Incorrect size on push, expected %u, got %zu\n", i + 1, scc_vec_size(*vec));
            return false;
        }
        if((*vec)[i + initsize] != data[i]) {
            fprintf(stderr, "Value mismatch for operator[] at index %u: %" PRIu32 " != %" PRIu32 "\n", i, (*vec)[i + initsize], data[i]);
            return false;
        }
        if(scc_vec_at(*vec, i + initsize) != data[i]) {
            fprintf(stderr, "Value mismatch for scc_vec_at at index %u: %" PRIu32 " != %" PRIu32 "\n", i, (*vec)[i + initsize], data[i]);
            return false;
        }
    }

    for(unsigned i = size; i > 0; --i) {
        if(size & 1) {
            scc_vec_pop(*vec);
        }
        else {
            scc_vec_pop_safe(*vec);
        }
        if(scc_vec_size(*vec) != i - 1ull + initsize) {
            fprintf(stderr, "Incorrect size on pop, expected %u, got %zu\n", i - 1, scc_vec_size(*vec));
            return false;
        }
    }

    return true;
}

static bool vec_fuzz_push_pop_test_reserve(uint32_t const *data, size_t size) {
    scc_vec(uint32_t) vec = scc_vec_new(uint32_t);
    if(!scc_vec_reserve(&vec, size)) {
        fputs("Allocation failure on reserve\n", stderr);
        return false;
    }
    size_t expcap = size < SCC_VEC_STATIC_CAPACITY ? SCC_VEC_STATIC_CAPACITY : size;

    if(scc_vec_capacity(vec) != expcap) {
        fprintf(stderr, "Unexpected capacity %zu instead of %zu\n", scc_vec_capacity(vec), expcap);
        return false;
    }

    bool success = vec_fuzz_push_pop_test(&vec, data, size, 0ull);

    scc_vec_free(vec);
    return success;
}

static bool vec_fuzz_push_pop_test_from(uint32_t const *data, size_t size) {
    if(size < 3) {
        return true;
    }

    uint32_t e0 = data[0];
    uint32_t e1 = data[1];
    uint32_t e2 = data[2];

    scc_vec(uint32_t) vec = scc_vec_from(uint32_t, e0, e1, e2);

    bool success = vec_fuzz_push_pop_test(&vec, &data[3], size - 3u, 3ull);
    scc_vec_free(vec);
    return success;
}

bool vec_fuzz_push_pop(uint32_t const *data, size_t size) {
    return vec_fuzz_push_pop_test_reserve(data, size) &&
           vec_fuzz_push_pop_test_from(data, size);
}
