#include <fuzzer/assertion.h>

#include "hashtab_fuzz_removal.h"

#include <inttypes.h>
#include <stdio.h>

void hashtab_fuzz_removal(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t unique_end,
    size_t size
) {
    uint32_t const *elem;
    for(size_t i = 0u; i < unique_end; ++i) {
        fuzz_assert(scc_hashtab_remove(*tab, data[i]), "Error removing %" PRIu32, data[i]);
        fuzz_assert(scc_hashtab_size(*tab) == unique_end - i - 1u,
                    "Incorrect size on removal, expected %zu, got %zu", unique_end - i - 1u, scc_hashtab_size(*tab));

        for(size_t j = i + 1u; j < unique_end; ++j) {
            elem = scc_hashtab_find(*tab, data[j]);
            fuzz_assert(elem, "Error looking up %" PRIu32 " on removal", data[j]);
            fuzz_assert(*elem == data[j], "Error on find during removal, expected %" PRIu32 ", got %" PRIu32,
                                          data[j], *elem);
            fuzz_assert(!scc_hashtab_insert(tab, data[j]),
                        "Allowed to insert %" PRIu32 " before it was removed", data[j]);
        }
    }

    fuzz_assert(!scc_hashtab_size(*tab), "Table should be empty but has size %zu", scc_hashtab_size(*tab));

    for(size_t i = unique_end; i < size; ++i) {
        fuzz_assert(!scc_hashtab_remove(*tab, data[i]), "Managed to remove non-existent element %" PRIu32, data[i]);
    }
}
