#include "vec_fuzz_traversal.h"

#include <scc/vec.h>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool vec_fuzz_traversal(uint32_t const *data, size_t size) {
    bool success = false;
    scc_vec(uint32_t) vec = scc_vec_new(uint32_t);

    if(!scc_vec_resize(&vec, size)) {
        fputs("Resize error\n", stderr);
        goto epilogue;
    }
    /* For coverage */
    scc_vec_resize(&vec, size);

    if(scc_vec_size(vec) != size) {
        fprintf(stderr, "Incorrect resize, expected %zu, got %zu\n", size, scc_vec_size(vec));
        goto epilogue;
    }

    uint32_t *iter;
    unsigned i = 0;
    scc_vec_foreach(iter, vec) {
        if(*iter) {
            fprintf(stderr, "Element %u not zeroed\n", i);
            goto epilogue;
        }
        ++i;
    }

    scc_vec_clear(vec);
    if(!scc_vec_empty(vec)) {
        fprintf(stderr, "Expected empty vector, got size %zu\n", scc_vec_size(vec));
        goto epilogue;
    }

    scc_vec_resize(&vec, size);
    memcpy(vec, data, size * sizeof(*data));
    scc_vec_resize(&vec, size - 1);

    if(size > 1) {
        i = size - 2;
        scc_vec_foreach_reversed(iter, vec) {
            if(*iter != data[i]) {
                fprintf(stderr, "Value mismatch at index %u, expected %" PRIu32 " , got %" PRIu32 "\n", i, data[i], *iter);
                goto epilogue;
            }
            --i;
        }
    }

    success = true;
epilogue:
    scc_vec_free(vec);
    return success;
}
