#include <instrumentation/rng.hpp>
#include <instrumentation/types.h>

#include "hashtab_compat.h"
#include "find.hpp"

extern "C" void const *scc_hashtab_impl_find(void const *hashtab, size_t elemsize);
extern "C" bool scc_hashtab_impl_insert(void *hashtab, size_t elemsize);

static bm_type *tab;
static std::vector<bm_type> data;

void hashtab_find_setup(benchmark::State const& state) {
    tab = static_cast<bm_type *>(hashtab_new());
    if(!tab) {
        std::abort();
    }
    data = rng::shuffled_iota(state);
    for(auto const v : data) {
        *tab = v;
        scc_hashtab_impl_insert(&tab, sizeof(*tab));
    }
}

void hashtab_find_teardown(benchmark::State const&) noexcept {
    hashtab_free(tab);
}

void hashtab_find(benchmark::State& state) {
    for(auto _ : state) {
        for(auto const v : data) {
            *tab = v;
            scc_hashtab_impl_find(tab, sizeof(*tab));
        }
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
