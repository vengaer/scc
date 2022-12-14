#include "bm_insertion.hpp"
#include "bmhash.hpp"
#include "hashtab_insertion.h"
#include "rng.hpp"
#include "std_unordered_set_insertion.hpp"

#include <unordered_set>

static void *table;
static std::unordered_set<bm_type, fnv1a<bm_type>> *stdtable;

void BM_insertion_setup(benchmark::State const&) {
    table = hashtab_insertion_init();
}

void BM_insertion_teardown(benchmark::State const&) {
    hashtab_insertion_free(table);
}

void BM_insertion(benchmark::State& state) {
    auto const bmdata = rng::iota_shuffle(state);
    for(auto _ : state) {
        benchmark::DoNotOptimize(
            hashtab_insertion_benchmark(&table, bmdata.data(), bmdata.size())
        );
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}

void BM_negative_insertion(benchmark::State& state) {
    auto const bmdata = rng::iota_shuffle(state);
    hashtab_insertion_benchmark(&table, bmdata.data(), bmdata.size());
    for(auto _ : state) {
        benchmark::DoNotOptimize(
            hashtab_insertion_benchmark(&table, bmdata.data(), bmdata.size())
        );
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}

void BM_insertion_std_setup(benchmark::State const&) {
    stdtable = new ptrtype(stdtable){};
}

void BM_insertion_std_teardown(benchmark::State const&) {
    delete stdtable;
}

void BM_insertion_std(benchmark::State& state) {
    auto const bmdata = rng::iota_shuffle(state);
    for(auto _ : state) {
        benchmark::DoNotOptimize(
            std_unordered_set_insertion_benchmark(stdtable, bmdata.data(), bmdata.size())
        );
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}

void BM_negative_insertion_std(benchmark::State& state) {
    auto const bmdata = rng::iota_shuffle(state);
    std_unordered_set_insertion_benchmark(stdtable, bmdata.data(), bmdata.size());
    for(auto _ : state) {
        benchmark::DoNotOptimize(
            std_unordered_set_insertion_benchmark(stdtable, bmdata.data(), bmdata.size())
        );
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
