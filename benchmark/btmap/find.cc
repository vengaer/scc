#include <instrumentation/rng.hpp>
#include <instrumentation/types.h>

#include "btmap_compat.h"
#include "find.hpp"

extern "C" void const *scc_btmap_impl_find(void const *btmap);
extern "C" bool scc_btmap_impl_insert(void *btmap);

static struct {
    bm_type key;
    bm_type value;
} *map;
static std::vector<bm_type> data;

void btmap_find_setup(benchmark::State const& state) {
    map = static_cast<decltype(map)>(btmap_new());
    if(!map) {
        std::abort();
    }
    data = rng::shuffled_iota(state);
    for(auto const v : data) {
        map->key = v;
        map->key = v + 1;
        scc_btmap_impl_insert(&map);
    }
}

void btmap_find_teardown(benchmark::State const&) noexcept {
    btmap_free(map);
}

void btmap_find(benchmark::State& state) {
    for(auto _ : state) {
        for(auto const v : data) {
            map->key = v;
            scc_btmap_impl_find(map);
        }
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
