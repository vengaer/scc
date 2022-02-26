#ifndef STD_UNORDERED_SET_FIND_HPP
#define STD_UNORDERED_SET_FIND_HPP

#include "bmtypes.h"
#include "bmhash.hpp"

#include <cstddef>
#include <unordered_set>

void std_unordered_set_find_insert_all(
    std::unordered_set<bm_type, fnv1a<bm_type>> *set,
    bm_type const *data,
    size_t size
);
void std_unordered_set_find_clear(std::unordered_set<bm_type, fnv1a<bm_type>> *set);
bool std_unordered_set_find_benchmark(
    std::unordered_set<bm_type, fnv1a<bm_type>> const *set,
    bm_type const *data,
    size_t size
);

#endif /* STD_UNORDERED_SET_FIND_HPP */
