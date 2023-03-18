#include <benchmark/benchmark.h>

#include "find.hpp"
#include "insert.hpp"

BENCHMARK(hashtab_insert)->
    Range(4, 4 << 16)->
    Setup(hashtab_insert_setup)->
    Teardown(hashtab_insert_teardown);

BENCHMARK(hashtab_find)->
    Range(4, 4 << 16)->
    Setup(hashtab_find_setup)->
    Teardown(hashtab_find_teardown);

BENCHMARK_MAIN();
