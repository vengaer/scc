#include "hashmap_fuzz_insertion.h"
#include "hashmap_fuzz_removal.h"

#include <fuzzer/assertion.h>
#include <fuzzer/dbg.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned partition(uint32_t *data, size_t size) {
    bool unique = true;
    unsigned unique_end = 0u;
    /* Stupid partitioning */
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

static bool eq(void const *left, void const *right) {
    return *(uint32_t const *)left == *(uint32_t const *)right;
}

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) {
    size /= sizeof(uint32_t);
    if(!size) {
        return 0;
    }
    bool success = false;
    scc_hashmap(uint32_t, uint16_t) map = scc_hashmap_new(uint32_t, uint16_t, eq);
    uint16_t *vals = 0;
    uint32_t *keys = malloc(size * sizeof(*keys));
    if(!keys) {
        fputs("key malloc failure\n", stderr);
        goto epilogue;
    }

    vals = malloc(size * sizeof(*vals));
    if(!vals) {
        fputs("value malloc failure\n", stderr);
        goto epilogue;
    }

    memcpy(keys, data, size * sizeof(*keys));
    memcpy(vals, data, size * sizeof(*vals));

    unsigned end = partition(keys, size);

    uint32_t *kit;
    dbg_pr("Keys:\n");
    dbg_pr_n(kit, keys, size, "%#08" PRIx32 " ", *kit);
    dbg_pr("\n");

    uint16_t *vit;
    dbg_pr("Values:\n");
    dbg_pr_n(vit, vals, size, "%#04" PRIx32 " ", *vit);
    dbg_pr("\n");

    for(unsigned i = 0u; i < end; ++i) {
        fuzz_assert(scc_hashmap_insert(&map, keys[i], vals[i]),
            "Could not insert { %" PRIu32 ", %" PRIu16 "}", keys[i], vals[i]);
        fuzz_assert(scc_hashmap_remove(map, keys[i]),
            "Could not remove { %" PRIu32 ", %" PRIu16 "}", keys[i], vals[i]);
    }

    hashmap_fuzz_insertion(&map, keys, vals, end, size);
    hashmap_fuzz_removal(&map, keys, end, size);

    success = true;
epilogue:
    free(keys);
    free(vals);
    scc_hashmap_free(map);
    if(!success) {
        abort();
    }
    return 0;
}
