#include <benchmark/benchmark.h>

#include "find.hpp"
#include "insert.hpp"

BENCHMARK(btmap_insert)->
    Range(4, 4 << 16)->
    Setup(btmap_insert_setup)->
    Teardown(btmap_insert_teardown);

BENCHMARK(btmap_find)->
    Range(4, 4 << 16)->
    Setup(btmap_find_setup)->
    Teardown(btmap_find_teardown);

BENCHMARK_MAIN();
