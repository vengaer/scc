#include "bm_creation.hpp"
#include "bm_find.hpp"
#include "bm_insertion.hpp"

#include <benchmark/benchmark.h>

BENCHMARK(BM_create);

BENCHMARK(BM_find)->
    Range(8, 8 << 20)->
    Setup(BM_find_unordered_setup)->
    Teardown(BM_find_teardown);

BENCHMARK(BM_insertion)->
    Range(8, 8 << 20)->
    Setup(BM_insertion_unordered_setup)->
    Teardown(BM_insertion_teardown);

BENCHMARK(BM_find_std)->
    Range(8, 8 << 20)->
    Setup(BM_find_std_unordered_setup)->
    Teardown(BM_find_std_teardown);

BENCHMARK(BM_insertion_std)->
    Range(8, 8 << 20)->
    Setup(BM_insertion_std_unordered_setup)->
    Teardown(BM_insertion_std_teardown);

BENCHMARK_MAIN();
