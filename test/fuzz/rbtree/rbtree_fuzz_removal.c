#include <inspect/scc_rbtree_inspect.h>

#include "rbtree_fuzz_removal.h"

#include <inttypes.h>
#include <stdio.h>

bool rbtree_fuzz_removal(scc_rbtree(uint32_t) *handle, uint32_t *data, size_t size) {
    unsigned long long flags;
    for(unsigned i = 0u; i < size; i++) {
        if(scc_rbtree_size(*handle) != size - i) {
            fprintf(stderr, "Size error, expected %zu, got %zu\n", size - i, scc_rbtree_size(*handle));
            return false;
        }

        if(!scc_rbtree_remove(*handle, data[i])) {
            fprintf(stderr, "Removal error, %" PRIu32 " not in tree\n", data[i]);
            return false;
        }

        for(unsigned j = i + 1u; j < size; j++) {
            if(!scc_rbtree_find(*handle, data[j])) {
                fprintf(stderr, "Value at index %u lost during removal: %" PRIu32 "\n", j, data[j]);
                return false;
            }
        }

        flags = scc_rbtree_inspect_properties(*handle);
        if(flags & SCC_RBTREE_ERR_MASK) {
            fprintf(stderr, "Property violation on removal (%#llx)\n", flags);
            scc_rbtree_inspect_dump_flags(flags);
            return false;
        }
    }

    for(unsigned i = 0; i < size; i++) {
        if(scc_rbtree_find(*handle, data[i])) {
            fprintf(stderr, "False positive for %" PRIu32 "on find\n", data[i]);
            return false;
        }
    }

    if(scc_rbtree_size(*handle)) {
        fprintf(stderr, "Size reported as %zu for empty tree\n", scc_rbtree_size(handle));
        return false;
    }
    return true;
}
