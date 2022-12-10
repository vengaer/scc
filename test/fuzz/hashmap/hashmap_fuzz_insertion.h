#ifndef HASHMAP_FUZZ_INSERTION_H
#define HASHMAP_FUZZ_INSERTION_H

#include <scc/hashmap.h>

#include <stddef.h>
#include <stdint.h>

void hashmap_fuzz_insertion(
    void *map,
    uint32_t const *restrict keys,
    uint16_t const *restrict vals,
    size_t unique_end,
    size_t size
);

#endif /* HASHMAP_FUZZ_INSERTION_H */
