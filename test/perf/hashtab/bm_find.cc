#include "bm_find.hpp"
#include "rng.hpp"

extern "C" {
    void *hashtab_find_init();
    void hashtab_find_free(void *table);
    void hashtab_find_insert_all(void **table, bm_type const *data, size_t size);
    void hashtab_find_clear(void *table);
    bool hashtab_find_benchmark(void *table, bm_type const *data, size_t size);
}

static void *table;

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
    state.PauseTiming();
    hashtab_find_clear(table);
    state.ResumeTiming();
}

void BM_find_setup(benchmark::State const&) {
    table = hashtab_find_init();
}

void BM_find_teardown(benchmark::State const&) {
    hashtab_find_free(table);
}
