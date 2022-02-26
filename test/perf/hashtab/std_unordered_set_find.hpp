#ifndef STD_UNORDERED_SET_FIND_HPP
#define STD_UNORDERED_SET_FIND_HPP

#include "bmtypes.h"

#include <cstddef>
#include <unordered_set>

void std_unordered_set_find_insert_all(std::unordered_set<bm_type> *unordered_set, bm_type const *data, size_t size);
void std_unordered_set_find_clear(std::unordered_set<bm_type> *unordered_set);
bool std_unordered_set_find_benchmark(std::unordered_set<bm_type> const *unordered_set, bm_type const *data, size_t size);

#endif /* STD_UNORDERED_SET_FIND_HPP */
