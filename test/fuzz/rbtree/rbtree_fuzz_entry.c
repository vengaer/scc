#include <fuzzer/dbg.h>
#include <inspect/rbtree_inspect.h>
#include <scc/rbtree.h>

#include "rbtree_fuzz_insertion.h"
#include "rbtree_fuzz_removal.h"
#include "rbtree_fuzz_traversal.h"

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
    /* The most stupid partitioning scheme ever */
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

int compare(void const *left, void const *right) {
    uint32_t leftval = *(uint32_t const *)left;
    uint32_t rightval = *(uint32_t const *)right;
    return (leftval < rightval ? -1 : 1) * !(leftval == rightval);
}

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) {
    uint32_t *buf = 0;
    size = size / sizeof(*buf);
    if(size < 1) {
        return 0;
    }

    buf = malloc(size * sizeof(*buf));
    if(!buf) {
        abort();
    }

    memcpy(buf, data, size * sizeof(*buf));
    unsigned unique_end = partition(buf, size);

    uint32_t const *it = 0;
    dbg_pr("Data:\n");
    dbg_pr_n(it, buf, size, "%" PRIu32 " ", *it);
    dbg_pr("\n");

    scc_rbtree(uint32_t) handle = scc_rbtree_new(uint32_t, compare);

    rbtree_fuzz_insert(&handle, buf, unique_end, size);
    rbtree_fuzz_traversal(handle, buf, unique_end);
    rbtree_fuzz_removal(handle, buf, unique_end);

    free(buf);
    scc_rbtree_free(handle);

    return 0;
}
