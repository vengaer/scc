#include "vec_fuzz_erase.h"
#include "vec_fuzz_push_pop.h"
#include "vec_fuzz_traversal.h"

#include <scc/vec.h>

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

    if(!vec_fuzz_push_pop(buf, size)) {
        goto epilogue;
    }

    if(!vec_fuzz_erase(buf, size)) {
        goto epilogue;
    }

    if(!vec_fuzz_traversal(buf, size)) {
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
