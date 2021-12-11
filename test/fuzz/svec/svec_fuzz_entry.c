#include "svec_fuzz_erase.h"
#include "svec_fuzz_push_pop.h"
#include "svec_fuzz_traversal.h"

#include <scc/scc_svec.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) {
    bool success = false;
    uint32_t *buf;
    size = size / sizeof(*buf);
    if(!size) {
        return 0;
    }
    buf = malloc(size * sizeof(*buf));
    if(!buf) {
        abort();
    }

    memcpy(buf, data, size * sizeof(*buf));

    if(!svec_fuzz_push_pop(buf, size)) {
        goto epilogue;
    }

    if(!svec_fuzz_erase(buf, size)) {
        goto epilogue;
    }

    if(!svec_fuzz_traversal(buf, size)) {
        goto epilogue;
    }

    success = true;
epilogue:
    free(buf);
    if(!success) {
        abort();
    }

    return 0;
}
