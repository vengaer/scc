#include "bm_insertion.hpp"
#include "rng.hpp"

extern "C" {
    void *hashtab_insertion_init();
    void hashtab_insertion_free(void *table);
    bool hashtab_insertion_benchmark(void **table, bm_type const* data, size_t size);
}

static void *table;

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

void BM_insertion_setup(benchmark::State const&) {
    table = hashtab_insertion_init();
}

void BM_insertion_teardown(benchmark::State const&) {
    hashtab_insertion_free(table);
}
