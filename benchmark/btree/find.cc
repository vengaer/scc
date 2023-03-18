#include <instrumentation/rng.hpp>
#include <instrumentation/types.h>

#include "btree_compat.h"
#include "find.hpp"

extern "C" void const *scc_btree_impl_find(void const *btree, size_t elemsize);
extern "C" bool scc_btree_impl_insert(void *btree, size_t elemsize);

static bm_type *tree;
static std::vector<bm_type> data;

void btree_find_setup(benchmark::State const& state) {
    tree = static_cast<bm_type *>(btree_new());
    if(!tree) {
        std::abort();
    }
    data = rng::shuffled_iota(state);
    for(auto const v : data) {
        *tree = v;
        scc_btree_impl_insert(&tree, sizeof(*tree));
    }
}

void btree_find_teardown(benchmark::State const&) noexcept {
    btree_free(tree);
}

void btree_find(benchmark::State& state) {
    for(auto _ : state) {
        for(auto const v : data) {
            *tree = v;
            scc_btree_impl_find(tree, sizeof(*tree));
        }
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
