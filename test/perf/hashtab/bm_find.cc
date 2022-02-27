#include "bm_find.hpp"
#include "bmhash.hpp"
#include "bmtypes.h"
#include "hashtab_find.h"
#include "rng.hpp"
#include "std_unordered_set_find.hpp"

#include <unordered_set>
#include <type_traits>

static void *table;
static std::unordered_set<bm_type, fnv1a<bm_type>> *stdtable;

void BM_find(benchmark::State& state) {
    auto const bmdata = rng::iota_shuffle(state);
    hashtab_find_insert_all(&table, bmdata.data(), bmdata.size());

    for(auto _ : state) {
        benchmark::DoNotOptimize(
            hashtab_find_benchmark(table, bmdata.data(), bmdata.size())
        );
    }

    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
    hashtab_find_clear(table);
}

void BM_find_setup(benchmark::State const&) {
    table = hashtab_find_init();
}

void BM_find_teardown(benchmark::State const&) {
    hashtab_find_free(table);
}

void BM_find_std(benchmark::State& state) {
    auto const bmdata = rng::iota_shuffle(state);
    std_unordered_set_find_insert_all(stdtable, bmdata.data(), bmdata.size());

    for(auto _ : state) {
        benchmark::DoNotOptimize(
            std_unordered_set_find_benchmark(stdtable, bmdata.data(), bmdata.size())
        );
    }

    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
    std_unordered_set_find_clear(stdtable);
}

void BM_find_std_setup(benchmark::State const&) {
    stdtable = new ptrtype(stdtable){};
}

void BM_find_std_teardown(benchmark::State const&) {
    delete stdtable;
}
