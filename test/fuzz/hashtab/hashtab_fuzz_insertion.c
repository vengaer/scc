#include <fuzzer/assertion.h>

#include "hashtab_fuzz_insertion.h"

#include <inttypes.h>
#include <stdio.h>

void hashtab_fuzz_insertion(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t unique_end,
    size_t size
) {
    uint32_t const *elem;
    for(unsigned i = 0u; i < unique_end; ++i) {
        fuzz_assert(scc_hashtab_insert(tab, data[i]), "Error inserting %" PRIu32, data[i]);
        fuzz_assert(scc_hashtab_size(*tab) == i + 1u,
                    "Incorrect size on insertion, expected %u, got %zu", i + 1u, scc_hashtab_size(*tab));

        for(unsigned j = 0u; j <= i; ++j) {
            elem = scc_hashtab_find(*tab, data[j]);
            fuzz_assert(elem, "%" PRIu32 " lost when inserting %" PRIu32, data[j], data[i]);
            fuzz_assert(*elem == data[j], "Incorrect element found on insertion, "
                                          "expected %" PRIu32 ", got %" PRIu32, data[j], *elem);
        }
    }

    for(unsigned i = unique_end; i < size; ++i) {
        fuzz_assert(!scc_hashtab_insert(tab, data[i]), "Successfully inserted %" PRIu32 " already in table", data[i]);
        fuzz_assert(scc_hashtab_size(*tab) == unique_end,
                    "Incorrect size on insertion failure, expected %zu, got %zu", unique_end, scc_hashtab_size(*tab));
        for(unsigned j = 0u; j < unique_end; ++j) {
            elem = scc_hashtab_find(*tab, data[j]);
            fuzz_assert(elem, "Lost %" PRIu32 " on insertion failure", data[j]);
            fuzz_assert(*elem == data[j], "Incorrect element found on insertion failure, "
                                          "expected %" PRIu32 ", got %" PRIu32, data[j], *elem);
        }
    }

}
