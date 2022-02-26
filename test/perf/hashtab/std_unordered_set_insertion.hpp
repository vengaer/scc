#ifndef STD_UNORDERED_SET_INSERTION_HPP
#define STD_UNORDERED_SET_INSERTION_HPP

#include "bmtypes.h"
#include "bmhash.hpp"

#include <cstddef>
#include <unordered_set>

bool std_unordered_set_insertion_benchmark(
    std::unordered_set<bm_type, fnv1a<bm_type>> *handle,
    bm_type const *data,
    size_t size
);

#endif /* STD_UNORDERED_SET_INSERTION_HPP */
