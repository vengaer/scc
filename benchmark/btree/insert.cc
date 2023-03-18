#include <instrumentation/rng.hpp>
#include <instrumentation/types.h>

#include "btree_compat.h"
#include "insert.hpp"

#include <cstddef>
#include <cstdlib>

extern "C" bool scc_btree_impl_insert(void *addr, size_t size);

static bm_type *tree;

void btree_insert_setup(benchmark::State const&) noexcept {
    tree = static_cast<bm_type *>(btree_new());
    if(!tree) {
        std::abort();
    }
}

void btree_insert_teardown(benchmark::State const&) noexcept {
    btree_free(tree);
}

void btree_insert(benchmark::State& state) {
    auto const data = rng::shuffled_iota(state);
    for(auto _ : state) {
        for(auto const v : data) {
            *tree = v;
            scc_btree_impl_insert(&tree, sizeof(*tree));
        }
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
