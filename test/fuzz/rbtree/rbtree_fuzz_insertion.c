#include <inspect/scc_rbtree_inspect.h>

#include "rbtree_fuzz_insertion.h"

#include <inttypes.h>
#include <stdio.h>

static bool rbtree_fuzz_verify_insert(scc_rbtree(uint32_t) *handle, uint32_t *data, size_t size, bool should_succeed) {
    unsigned long long flags;
    size_t const tree_size = scc_rbtree_size(*handle);
    size_t expected_size;
    for(unsigned i = 0u; i < size; ++i) {
        expected_size = should_succeed ? i : tree_size;
        if(scc_rbtree_size(*handle) != expected_size) {
            fprintf(stderr, "Incorrect size, expected %zu, got %zu\n", expected_size, scc_rbtree_size(*handle));
            return false;
        }

        if(scc_rbtree_insert(handle, data[i]) != should_succeed) {
            fprintf(stderr, "Insertion error, expected %s, got %s\n", should_succeed ? "success" : "failure",
                                                                      should_succeed ? "failure" : "success");
            return false;
        }

        for(unsigned j = 0u; j < i; ++j) {
            if(!scc_rbtree_find(*handle, data[j])) {
                fprintf(stderr, "Value at index %u lost: %" PRIu32 "\n", j, data[j]);
                return false;
            }
        }

        flags = scc_rbtree_inspect_properties(*handle);
        if(flags & SCC_RBTREE_ERR_MASK) {
            fprintf(stderr, "Property violation on insertion (%#llx)\n", flags);
            scc_rbtree_inspect_dump_flags(flags);
            return false;
        }
    }
    return true;
}

bool rbtree_fuzz_insert(scc_rbtree(uint32_t) *handle, uint32_t *data, unsigned unique_end, size_t size) {
    if(!rbtree_fuzz_verify_insert(handle, data, unique_end, true)) {
        return false;
    }
    return rbtree_fuzz_verify_insert(handle, data + unique_end, size - unique_end, false);
}
