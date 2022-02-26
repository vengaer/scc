#include "std_unordered_set_insertion.hpp"

bool std_unordered_set_insertion_benchmark(
    std::unordered_set<bm_type, fnv1a<bm_type>> *set,
    bm_type const *data,
    size_t size
) {
    bool all_inserted = true;
    for(size_t i = 0u; i < size; ++i) {
        all_inserted &= !!set->insert(data[i]).second;
    }
    return all_inserted;
}
