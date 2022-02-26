#include "bm_creation.hpp"
#include "bm_find.hpp"
#include "bm_insertion.hpp"

#include <benchmark/benchmark.h>

BENCHMARK(BM_create);

BENCHMARK(BM_find)->
    Range(8, 8 << 20)->
    Setup(BM_find_setup)->
    Teardown(BM_find_teardown);

BENCHMARK(BM_insertion)->
    Range(8, 8 << 20)->
    Setup(BM_insertion_setup)->
    Teardown(BM_insertion_teardown);

BENCHMARK_MAIN();
