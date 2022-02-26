#include "std_unordered_set_insertion.hpp"

#include <unordered_set>

bool std_unordered_set_insertion_benchmark(void *handle, bm_type const *data, size_t size) {
    std::unordered_set<bm_type> *unordered_set = static_cast<std::unordered_set<bm_type>*>(handle);
    bool all_inserted = true;
    for(size_t i = 0u; i < size; ++i) {
        all_inserted &= !!unordered_set->insert(data[i]).second;
    }
    return all_inserted;
}
