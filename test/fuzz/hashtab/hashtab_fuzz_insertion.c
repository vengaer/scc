#include "hashtab_fuzz_insertion.h"

#include <inttypes.h>
#include <stdio.h>

bool hashtab_fuzz_insertion(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t unique_end,
    size_t size
) {
    for(unsigned i = 0u; i < unique_end; ++i) {
        if(!scc_hashtab_insert(*tab, data[i])) {
            fprintf(stderr, "Error inserting %" PRIu32 "\n", data[i]);
            return false;
        }

        for(unsigned j = 0u; j <= i; ++j) {
            if(!scc_hashtab_find(*tab, data[j])) {
                fprintf(stderr, "%" PRIu32 " lost when inserting %" PRIu32 "\n", data[j], data[i]);
                return false;
            }
        }
    }

    for(unsigned i = unique_end; i < size; ++i) {
        if(scc_hashtab_insert(*tab, data[i])) {
            fprintf(stderr, "Successfully inserted %" PRIu32 " already present in table\n", data[i]);
            return false;
        }
        for(unsigned j = 0u; j < unique_end; ++j) {
            if(!scc_hashtab_find(*tab, data[j])) {
                fprintf(stderr, "Lost %" PRIu32 " on insertion failure\n", data[j]);
                return false;
            }
        }
    }
    return true;
}
