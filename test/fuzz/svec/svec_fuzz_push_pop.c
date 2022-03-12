#include "svec_fuzz_push_pop.h"

#include <scc/scc_svec.h>

#include <inttypes.h>
#include <stdio.h>

static bool svec_fuzz_push_pop_test(scc_svec(uint32_t) *vec, uint32_t const *data, size_t size, size_t initsize) {
    for(unsigned i = 0; i < size; ++i) {
        if(!scc_svec_push(vec, data[i])) {
            fprintf(stderr, "Insertion error at element %u\n", i);
            return false;
        }
        if(scc_svec_size(*vec) != i + 1ull + initsize) {
            fprintf(stderr, "Incorrect size on push, expected %u, got %zu\n", i + 1, scc_svec_size(*vec));
            return false;
        }
        if((*vec)[i + initsize] != data[i]) {
            fprintf(stderr, "Value mismatch for operator[] at index %u: %" PRIu32 " != %" PRIu32 "\n", i, (*vec)[i + initsize], data[i]);
            return false;
        }
        if(scc_svec_at(*vec, i + initsize) != data[i]) {
            fprintf(stderr, "Value mismatch for scc_svec_at at index %u: %" PRIu32 " != %" PRIu32 "\n", i, (*vec)[i + initsize], data[i]);
            return false;
        }
    }

    for(unsigned i = size; i > 0; --i) {
        if(size & 1) {
            scc_svec_pop(*vec);
        }
        else {
            scc_svec_pop_safe(*vec);
        }
        if(scc_svec_size(*vec) != i - 1ull + initsize) {
            fprintf(stderr, "Incorrect size on pop, expected %u, got %zu\n", i - 1, scc_svec_size(*vec));
            return false;
        }
    }

    return true;
}

static bool svec_fuzz_push_pop_test_reserve(uint32_t const *data, size_t size) {
    scc_svec(uint32_t) svec = scc_svec_init(uint32_t);
    if(!scc_svec_reserve(&svec, size)) {
        fputs("Allocation failure on reserve\n", stderr);
        return false;
    }
    size_t expcap = size < SCC_SVEC_STATIC_CAPACITY ? SCC_SVEC_STATIC_CAPACITY : size;

    if(scc_svec_capacity(svec) != expcap) {
        fprintf(stderr, "Unexpected capacity %zu instead of %zu\n", scc_svec_capacity(svec), expcap);
        return false;
    }

    bool success = svec_fuzz_push_pop_test(&svec, data, size, 0ull);

    scc_svec_free(svec);
    return success;
}

static bool svec_fuzz_push_pop_test_from(uint32_t const *data, size_t size) {
    if(size < 3) {
        return true;
    }

    uint32_t e0 = data[0];
    uint32_t e1 = data[1];
    uint32_t e2 = data[2];

    scc_svec(uint32_t) svec = scc_svec_from(uint32_t, e0, e1, e2);

    bool success = svec_fuzz_push_pop_test(&svec, &data[3], size - 3u, 3ull);
    scc_svec_free(svec);
    return success;
}

bool svec_fuzz_push_pop(uint32_t const *data, size_t size) {
    return svec_fuzz_push_pop_test_reserve(data, size) &&
           svec_fuzz_push_pop_test_from(data, size);
}
