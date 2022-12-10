#ifndef HASHTAB_FUZZ_REMOVAL_H
#define HASHTAB_FUZZ_REMOVAL_H

#include <scc/hashtab.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void hashtab_fuzz_removal(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t unique_end,
    size_t size
);

#endif /* HASHTAB_FUZZ_REMOVAL_H */
