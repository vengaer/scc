#include <benchmark/benchmark.h>

#include "find.hpp"
#include "insert.hpp"

BENCHMARK(btree_insert)->
    Range(4, 4 << 16)->
    Setup(btree_insert_setup)->
    Teardown(btree_insert_teardown);

BENCHMARK(btree_find)->
    Range(4, 4 << 16)->
    Setup(btree_find_setup)->
    Teardown(btree_find_teardown);

BENCHMARK_MAIN();
