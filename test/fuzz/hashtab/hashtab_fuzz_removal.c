#include "hashtab_fuzz_removal.h"

#include <inttypes.h>
#include <stdio.h>

bool hashtab_fuzz_removal(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t unique_end,
    size_t size
) {
    uint32_t const *elem;
    for(size_t i = 0u; i < unique_end; ++i) {
        if(!scc_hashtab_remove(*tab, data[i])) {
            fprintf(stderr, "Error removing %" PRIu32 "\n", data[i]);
            return false;
        }

        if(scc_hashtab_size(*tab) != unique_end - i - 1u) {
            fprintf(stderr, "Incorrect size on removal, expected %zu, got %zu\n", unique_end - i - 1u, scc_hashtab_size(*tab));
            return false;
        }

        for(size_t j = i + 1u; j < unique_end; ++j) {
            elem = scc_hashtab_find(*tab, data[j]);
            if(!elem) {
                fprintf(stderr, "Error looking up %" PRIu32 " on removal\n", data[j]);
                return false;
            }
            if(*elem != data[j]) {
                fprintf(stderr, "Error on find during removal, expected %" PRIu32 ", got %" PRIu32 "\n", data[j], *elem);
                return false;
            }
            if(scc_hashtab_insert(tab, data[j])) {
                fprintf(stderr, "Allowed to insert %" PRIu32 " before it was removed\n", data[j]);
                return false;
            }
        }
    }

    if(scc_hashtab_size(*tab)) {
        fprintf(stderr, "Table should be empty, got size %zu\n", scc_hashtab_size(*tab));
        return false;
    }

    for(size_t i = unique_end; i < size; ++i) {
        if(scc_hashtab_remove(*tab, data[i])) {
            fprintf(stderr, "Managed to remove non-existent element %" PRIu32 "\n", data[i]);
            return false;
        }
    }

    return true;
}
