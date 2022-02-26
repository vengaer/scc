#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef SCC_BM_TYPE
#error SCC_BM_TYPE not defined
#endif

using bm_type = SCC_BM_TYPE;
static_assert(std::is_arithmetic_v<bm_type>);

extern "C" void insertion_benchmark(bm_type const* data, size_t size);
extern "C" void automatic_init_free_benchmark();

static void BM_insert(benchmark::State& state) {
    std::vector<bm_type> bmdata{};
    bmdata.resize(state.range(0));
    std::iota(std::begin(bmdata), std::end(bmdata), bm_type{});

    std::mt19937 rng{std::random_device{}()};
    std::shuffle(std::begin(bmdata), std::end(bmdata), rng);

    for(auto _ : state) {
        insertion_benchmark(bmdata.data(), bmdata.size());
    }
    state.SetBytesProcessed(static_cast<long long>(state.iterations()) *
                            static_cast<long long>(state.range(0)));
}
BENCHMARK(BM_insert)->Range(8, 8 << 20);

static void BM_create(benchmark::State& state) {
    for(auto _ : state) {
        automatic_init_free_benchmark();
    }
}
BENCHMARK(BM_create)->Range(8, 8 << 20);


BENCHMARK_MAIN();
