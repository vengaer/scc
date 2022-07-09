#include <inspect/scc_rbtree_inspect.h>
#include <scc/scc_rbtree.h>

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
    bool success = false;
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

    scc_rbtree(uint32_t) handle = scc_rbtree_new(uint32_t, compare);
    if(!handle) {
        goto epilogue;
    }

    if(!rbtree_fuzz_insert(&handle, buf, unique_end, size)) {
        goto epilogue;
    }

    if(!rbtree_fuzz_traversal(&handle, buf, unique_end)) {
        goto epilogue;
    }

    if(!rbtree_fuzz_removal(&handle, buf, unique_end)) {
        goto epilogue;
    }

    success = true;
epilogue:
    free(buf);
    if(handle) {
        scc_rbtree_free(handle);
    }
    if(!success) {
        abort();
    }

    return 0;
}
