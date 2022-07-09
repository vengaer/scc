#include "svec_fuzz_traversal.h"

#include <scc/scc_svec.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool svec_fuzz_traversal(uint32_t const *data, size_t size) {
    bool success = false;
    scc_svec(uint32_t) svec = scc_svec_new(uint32_t);

    if(!scc_svec_resize(svec, size)) {
        fputs("Resize error\n", stderr);
        goto epilogue;
    }
    /* For coverage */
    scc_svec_resize(svec, size);

    if(scc_svec_size(svec) != size) {
        fprintf(stderr, "Incorrect resize, expected %zu, got %zu\n", size, scc_svec_size(svec));
        goto epilogue;
    }

    uint32_t *iter;
    unsigned i = 0;
    scc_svec_foreach(iter, svec) {
        if(*iter) {
            fprintf(stderr, "Element %u not zeroed\n", i);
            goto epilogue;
        }
        ++i;
    }

    scc_svec_clear(svec);
    if(!scc_svec_empty(svec)) {
        fprintf(stderr, "Expected empty vector, got size %zu\n", scc_svec_size(svec));
        goto epilogue;
    }

    scc_svec_resize(svec, size);
    memcpy(svec, data, size * sizeof(*data));
    scc_svec_resize(svec, size - 1);

    if(size > 1) {
        i = size - 2;
        scc_svec_foreach_reversed(iter, svec) {
            if(*iter != data[i]) {
                fprintf(stderr, "Value mismatch at index %u, expected %" PRIu32 " , got %" PRIu32 "\n", i, data[i], *iter);
                goto epilogue;
            }
            --i;
        }
    }

    success = true;
epilogue:
    scc_svec_free(svec);
    return success;
}
