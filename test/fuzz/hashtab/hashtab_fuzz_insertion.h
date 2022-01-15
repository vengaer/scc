#ifndef HASHTAB_FUZZ_INSERTION_H
#define HASHTAB_FUZZ_INSERTION_H

#include <scc/scc_hashtab.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool hashtab_fuzz_insertion(
    scc_hashtab(uint32_t) *tab,
    uint32_t const *restrict data,
    size_t unique_end,
    size_t size
);

#endif /* HASHTAB_FUZZ_INSERTION_H */