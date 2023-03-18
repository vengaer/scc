#include <instrumentation/rng.hpp>
#include <instrumentation/types.h>

#include "hashtab_compat.h"
#include "insert.hpp"

#include <cstddef>
#include <cstdlib>

extern "C" bool scc_hashtab_impl_insert(void *addr, size_t size);

static bm_type *tab;
static std::vector<bm_type> data;

void hashtab_insert_setup(benchmark::State const& state) noexcept {
    tab = static_cast<bm_type *>(hashtab_new());
    if(!tab) {
        std::abort();
    }
    data = rng::shuffled_iota(state);
}

void hashtab_insert_teardown(benchmark::State const&) noexcept {
    hashtab_free(tab);
}

void hashtab_insert(benchmark::State& state) {
    for(auto _ : state) {
        for(auto const v : data) {
            *tab = v;
            scc_hashtab_impl_insert(&tab, sizeof(*tab));
        }
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
