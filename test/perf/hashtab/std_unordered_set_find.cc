#include "std_unordered_set_find.hpp"

void std_unordered_set_find_insert_all(
    std::unordered_set<bm_type, fnv1a<bm_type>> *set,
    bm_type const *data,
    size_t size
) {
    for(unsigned i = 0u; i < size; ++i) {
        set->insert(data[i]);
    }
}

void std_unordered_set_find_clear(std::unordered_set<bm_type, fnv1a<bm_type>> *set) {
    set->clear();
}

bool std_unordered_set_find_benchmark(
    std::unordered_set<bm_type, fnv1a<bm_type>> const *set,
    bm_type const *data,
    size_t size
) {
    bool all_found = true;
    for(unsigned i = 0u; i < size; ++i) {
        all_found &= set->find(data[i]) != set->end();
    }
    return all_found;
}
