#include <fuzzer/assertion.h>
#include <scc/hashmap.h>

#include "hashmap_fuzz_removal.h"

#include <inttypes.h>
#include <stdio.h>

void hashmap_fuzz_removal(
    void *map,
    uint32_t const *restrict keys,
    size_t unique_end,
    size_t size
) {
    scc_hashmap(uint32_t, uint16_t) *m = map;
    uint16_t const *elem;
    for(size_t i = 0u; i < unique_end; ++i) {
        fuzz_assert(scc_hashmap_remove(*m, keys[i]), "Error removing %" PRIu32, keys[i]);
        fuzz_assert(scc_hashmap_size(*m) == unique_end - i - 1u,
                    "Incorrect size on removal, expected %zu, got %zu", unique_end - i - 1u, scc_hashmap_size(*m));

        for(size_t j = i + 1u; j < unique_end; ++j) {
            elem = scc_hashmap_find(*m, keys[j]);
            fuzz_assert(!!elem, "Error looking up %" PRIu32 " on removal", keys[j]);
        }
    }

    fuzz_assert(!scc_hashmap_size(*m), "Map should be empty but has size %zu", scc_hashmap_size(*m));

    for(size_t i = unique_end; i < size; ++i) {
        fuzz_assert(!scc_hashmap_remove(*m, keys[i]), "Managed to remove non-existent element %" PRIu32, keys[i]);
    }
}
