#ifndef HASHTAB_FUZZ_PROBING_H
#define HASHTAB_FUZZ_PROBING_H

#include <scc/hashtab.h>

#include <stdint.h>

void hashtab_fuzz_probing(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t size
);

#endif /* HASHTAB_FUZZ_PROBING_H */
