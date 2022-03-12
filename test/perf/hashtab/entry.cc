#include "bm_creation.hpp"
#include "bm_find.hpp"
#include "bm_insertion.hpp"

#include <benchmark/benchmark.h>

BENCHMARK(BM_create);

BENCHMARK(BM_find)->
    Range(8, 8 << 20)->
    Setup(BM_find_setup)->
    Teardown(BM_find_teardown);

#ifdef SCC_BENCHMARK_STD
BENCHMARK(BM_find_std)->
    Range(8, 8 << 20)->
    Setup(BM_find_std_setup)->
    Teardown(BM_find_std_teardown);
#endif

BENCHMARK(BM_insertion)->
    Range(8, 8 << 20)->
    Setup(BM_insertion_setup)->
    Teardown(BM_insertion_teardown);

#ifdef SCC_BENCHMARK_STD
BENCHMARK(BM_insertion_std)->
    Range(8, 8 << 20)->
    Setup(BM_insertion_std_setup)->
    Teardown(BM_insertion_std_teardown);
#endif

BENCHMARK(BM_negative_insertion)->
    Range(8, 8 << 20)->
    Setup(BM_insertion_setup)->
    Teardown(BM_insertion_teardown);

#ifdef CONFIG_BENCHMARK_STD
BENCHMARK(BM_negative_insertion_std)->
    Range(8, 8 << 20)->
    Setup(BM_insertion_std_setup)->
    Teardown(BM_insertion_std_teardown);
#endif

BENCHMARK_MAIN();
