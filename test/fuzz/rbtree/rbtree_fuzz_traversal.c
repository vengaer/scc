#include "rbtree_fuzz_traversal.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int compare(void const *left, void const *right);


bool rbtree_fuzz_traversal(scc_rbtree(uint32_t) const *handle, uint32_t const *data, size_t size) {
    uint32_t *buf = malloc(size * sizeof(*buf));
    if(!buf) {
        fputs("malloc failure\n", stderr);
        return false;
    }

    memcpy(buf, data, size * sizeof(*buf));
    qsort(buf, size, sizeof(*buf), compare);

    uint32_t const *iter;
    unsigned i = 0u;
    scc_rbtree_foreach(iter, *handle) {
        if(*iter != buf[i]) {
            fprintf(stderr, "Element %u with value %" PRIu32 "out-of-order in traversal\n", i, buf[i]);
            return false;
        }
        ++i;
    }

    scc_rbtree_foreach_reversed(iter, *handle) {
        if(*iter != buf[--i]) {
            fprintf(stderr, "Element %u with value %" PRIu32 "out-of-order in reverse traversal\n", i, buf[i]);
            return false;
        }
    }

    free(buf);
    return true;
}
