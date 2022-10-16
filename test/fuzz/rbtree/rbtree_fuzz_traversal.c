#include <fuzzer/assertion.h>
#include "rbtree_fuzz_traversal.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int compare(void const *left, void const *right);

void rbtree_fuzz_traversal(scc_rbtree(uint32_t) const handle, uint32_t const *data, size_t size) {
    uint32_t *buf = malloc(size * sizeof(*buf));
    fuzz_assert(buf, "malloc failure");

    memcpy(buf, data, size * sizeof(*buf));
    qsort(buf, size, sizeof(*buf), compare);

    uint32_t const *iter;
    unsigned i = 0u;
    scc_rbtree_foreach(iter, handle) {
        fuzz_assert(*iter == buf[i], "Element %u with value %" PRIu32 " encountered out of order", i, buf[i]);
        ++i;
    }

    scc_rbtree_foreach_reversed(iter, handle) {
        fuzz_assert(*iter == buf[--i], "Element %u with value %" PRIu32 " encountered out of order", i, buf[i]);
    }

    free(buf);
}
