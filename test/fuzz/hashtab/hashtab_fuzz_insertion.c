#include "hashtab_fuzz_insertion.h"

#include <inttypes.h>
#include <stdio.h>

bool hashtab_fuzz_insertion(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t unique_end,
    size_t size
) {
    uint32_t const *elem;
    for(unsigned i = 0u; i < unique_end; ++i) {
        if(!scc_hashtab_insert(*tab, data[i])) {
            fprintf(stderr, "Error inserting %" PRIu32 "\n", data[i]);
            return false;
        }

        if(scc_hashtab_size(*tab) != i + 1u) {
            fprintf(stderr, "Incorrect size on insertion, expected %u, got %zu\n", i + 1u, scc_hashtab_size(*tab));
            return false;
        }

        for(unsigned j = 0u; j <= i; ++j) {
            elem = scc_hashtab_find(*tab, data[j]);
            if(!elem) {
                fprintf(stderr, "%" PRIu32 " lost when inserting %" PRIu32 "\n", data[j], data[i]);
                return false;
            }
            if(*elem != data[j]) {
                fprintf(stderr, "Incorrect element found on insertion, exptected %" PRIu32 ", got %" PRIu32 "\n", data[j], *elem);
                return false;
            }
        }
    }

    for(unsigned i = unique_end; i < size; ++i) {
        if(scc_hashtab_insert(*tab, data[i])) {
            fprintf(stderr, "Successfully inserted %" PRIu32 " already present in table\n", data[i]);
            return false;
        }
        if(scc_hashtab_size(*tab) != unique_end) {
            fprintf(stderr, "Incorrect size on insertion failure, expected %zu, got %zu\n", unique_end, scc_hashtab_size(*tab));
            return false;
        }
        for(unsigned j = 0u; j < unique_end; ++j) {
            elem = scc_hashtab_find(*tab, data[j]);
            if(!elem) {
                fprintf(stderr, "Lost %" PRIu32 " on insertion failure\n", data[j]);
                return false;
            }
            if(*elem != data[j]) {
                fprintf(stderr, "Incorrect element found on insertion failure, exptected %" PRIu32 ", got %" PRIu32 "\n", data[j], *elem);
                return false;
            }
        }
    }
    return true;
}
