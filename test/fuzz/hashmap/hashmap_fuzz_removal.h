#ifndef HASHMAP_FUZZ_REMOVAL_H
#define HASHMAP_FUZZ_REMOVAL_H

#include <stddef.h>
#include <stdint.h>

void hashmap_fuzz_removal(
    void *map,
    uint32_t const *restrict keys,
    size_t unique_end,
    size_t size
);

#endif /* HASHMAP_FUZZ_REMOVAL_H */
