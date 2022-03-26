#include <fuzzer/assertion.h>

#include "hashmap_fuzz_insertion.h"

#include <inttypes.h>
#include <stdio.h>

void hashmap_fuzz_insertion(
    void *map,
    uint32_t const *restrict keys,
    uint16_t const *restrict vals,
    size_t unique_end,
    size_t size
) {
    scc_hashmap(uint32_t, uint16_t) *m = map;
    uint16_t *elem;
    for(unsigned i = 0u; i < unique_end; ++i) {
        fuzz_assert(scc_hashmap_insert(m, keys[i], vals[i]),
                   "Error inserting { %" PRIu32 ", %" PRIu16 " }", keys[i], vals[i]);
        fuzz_assert(scc_hashmap_size(*m) == i + 1u,
                    "Incorrect size on insertion, expected %u, got %zu", i + 1u, scc_hashmap_size(*m));

        for(unsigned j = 0u; j <= i; ++j) {
            elem = scc_hashmap_find(*m, keys[j]);
            fuzz_assert(!!elem, "{ %" PRIu32 ", %" PRIu16 " } lost when inserting "
                                "{ %" PRIu32 ", %" PRIu16 " }", keys[j], vals[j], keys[i], vals[i]);
            fuzz_assert(*elem == vals[j], "Incorrect element found on insertion, "
                                          "expected %" PRIu16 ", got %" PRIu16, vals[j], *elem);
        }
    }

    uint16_t *old;
    for(unsigned i = unique_end; i < size; ++i) {
        old = scc_hashmap_find(*m, keys[i]);
        fuzz_assert(scc_hashmap_insert(m, keys[i], vals[i]),
                   "Could not overwrite value for key %" PRIu32, keys[i]);
        elem = scc_hashmap_find(*m, keys[i]);
        fuzz_assert(!!elem, "{ %" PRIu32 ", %" PRIu16 " } lost when overwriting entry", keys[i], vals[i]);
        fuzz_assert(elem == old, "Pointers returned for find %" PRIu32 " differ", keys[i]);
        fuzz_assert(scc_hashmap_size(*m) == unique_end,
                    "Incorrect size on override insertion, expected %zu, got %zu", unique_end, scc_hashmap_size(*m));
        for(unsigned j = 0u; j < unique_end; ++j) {
            elem = scc_hashmap_find(*m, keys[j]);
            fuzz_assert(elem, "Lost value for key %" PRIu32 " on insertion failure", keys[j]);
        }
    }
}
