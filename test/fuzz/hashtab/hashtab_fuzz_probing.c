#include <fuzzer/assertion.h>

#include <inttypes.h>

#include "hashtab_fuzz_probing.h"

void hashtab_fuzz_probing(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t size
) {
    for(size_t i = 0u; i < size; ++i) {
        fuzz_assert(scc_hashtab_insert(tab, data[i]),
                    "Could not insert %" PRIu32 " while probe fuzzing", data[i]);
        fuzz_assert(scc_hashtab_remove(*tab, data[i]),
                    "Could not remove %" PRIu32 " while probe fuzzing", data[i]);
    }
}
