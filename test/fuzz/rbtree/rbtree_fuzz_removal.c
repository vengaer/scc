#include <fuzzer/assertion.h>
#include <inspect/rbtree_inspect.h>

#include "rbtree_fuzz_removal.h"

#include <inttypes.h>
#include <stdio.h>

void rbtree_fuzz_removal(scc_rbtree(uint32_t) handle, uint32_t *data, size_t size) {
    unsigned long long flags;
    for(unsigned i = 0u; i < size; i++) {
        fuzz_assert(
            scc_rbtree_size(handle) == size - i,
            "Size error, expected %zu, got %zu", size - i, scc_rbtree_size(handle)
        );

        fuzz_assert(scc_rbtree_remove(handle, data[i]), "Removal error, %" PRIu32 " not in tree", data[i]);

        for(unsigned j = i + 1u; j < size; j++) {
            fuzz_assert(scc_rbtree_find(handle, data[j]), "Value at index %u lost during removal: %" PRIu32, j, data[j]);
        }

        flags = scc_rbtree_inspect_properties(handle);
        fuzz_assert(!(flags & SCC_RBTREE_ERR_MASK), "Property violation on removal (%#llx)", flags);
    }

    for(unsigned i = 0; i < size; i++) {
        fuzz_assert(!scc_rbtree_find(handle, data[i]), "False positive for %" PRIu32 " on find", data[i]);
    }

    fuzz_assert(!scc_rbtree_size(handle), "Size reported as %zu for empty tree", scc_rbtree_size(handle));
}
