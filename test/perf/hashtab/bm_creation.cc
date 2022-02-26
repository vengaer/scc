#include "bm_creation.hpp"

extern "C" bool automatic_init_free_benchmark();

void BM_create(benchmark::State& state) {
    for(auto _ : state) {
        benchmark::DoNotOptimize(
            automatic_init_free_benchmark()
        );
    }
}
