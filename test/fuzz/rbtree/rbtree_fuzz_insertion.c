#include <fuzzer/assertion.h>
#include <inspect/rbtree_inspect.h>

#include "rbtree_fuzz_insertion.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

static inline char const *bts(bool val) {
    if(val) {
        return "success";
    }
    return "failure";
}

static void rbtree_fuzz_verify_insert(scc_rbtree(uint32_t) *handle, uint32_t *data, size_t size, bool should_succeed) {
    unsigned long long flags;
    size_t const tree_size = scc_rbtree_size(*handle);
    size_t expected_size;
    for(unsigned i = 0u; i < size; ++i) {
        expected_size = should_succeed ? i : tree_size;
        fuzz_assert(
            scc_rbtree_size(*handle) == expected_size,
            "Incorrect size, expected %zu, got %zu", expected_size, scc_rbtree_size(*handle)
        );


        fuzz_assert(
            scc_rbtree_insert(handle, data[i]) == should_succeed,
            "Insertion error, expected %s, got %s\n", bts(should_succeed), bts(!should_succeed)
        );

        for(unsigned j = 0u; j < i; ++j) {
            fuzz_assert(
                scc_rbtree_find(*handle, data[j]),
                "Value at index %u lost: %" PRIu32, j, data[j]
            );
        }

        flags = scc_rbtree_inspect_properties(*handle);
        fuzz_assert(!(flags & SCC_RBTREE_ERR_MASK), "Property violation on insertion (%#llx)", flags);
    }
}

void rbtree_fuzz_insert(scc_rbtree(uint32_t) *handle, uint32_t *data, unsigned unique_end, size_t size) {
    rbtree_fuzz_verify_insert(handle, data, unique_end, true);
    rbtree_fuzz_verify_insert(handle, data + unique_end, size - unique_end, false);
}
