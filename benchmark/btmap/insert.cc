#include <instrumentation/rng.hpp>
#include <instrumentation/types.h>

#include "btmap_compat.h"
#include "insert.hpp"

#include <cstddef>
#include <cstdlib>

extern "C" bool scc_btmap_impl_insert(void *addr);

static struct {
    bm_type key;
    bm_type value;
} *map;
static std::vector<bm_type> data;

void btmap_insert_setup(benchmark::State const& state) noexcept {
    map = static_cast<decltype(map)>(btmap_new());
    if(!map) {
        std::abort();
    }
    data = rng::shuffled_iota(state);
}

void btmap_insert_teardown(benchmark::State const&) noexcept {
    btmap_free(map);
}

void btmap_insert(benchmark::State& state) {
    for(auto _ : state) {
        for(auto const v : data) {
            map->key = v;
            map->value = v + 1;
            scc_btmap_impl_insert(&map);
        }
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
