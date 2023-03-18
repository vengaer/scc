#include <benchmark/benchmark.h>

#include "insert.hpp"

BENCHMARK(btree_insert)->
    Range(4, 4 << 16)->
    Setup(btree_insert_setup)->
    Teardown(btree_insert_teardown);

BENCHMARK_MAIN();
