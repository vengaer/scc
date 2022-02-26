#include "std_unordered_set_find.hpp"

void std_unordered_set_find_insert_all(std::unordered_set<bm_type> *unordered_set, bm_type const *data, size_t size) {
    for(unsigned i = 0u; i < size; ++i) {
        unordered_set->insert(data[i]);
    }
}

void std_unordered_set_find_clear(std::unordered_set<bm_type> *unordered_set) {
    unordered_set->clear();
}

bool std_unordered_set_find_benchmark(std::unordered_set<bm_type> const *unordered_set, bm_type const *data, size_t size) {
    bool all_found = true;
    for(unsigned i = 0u; i < size; ++i) {
        all_found &= unordered_set->find(data[i]) != unordered_set->end();
    }
    return all_found;
}
