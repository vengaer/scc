#ifndef BM_FIND_HPP
#define BM_FIND_HPP

#include "bmtypes.h"

#include <benchmark/benchmark.h>

void BM_find(benchmark::State& state);
void BM_find_setup(benchmark::State const& state);
void BM_find_teardown(benchmark::State const& state);

void BM_find_std(benchmark::State& state);
void BM_find_std_setup(benchmark::State const& state);
void BM_find_std_teardown(benchmark::State const& state);

#endif /* BM_FIND_HPP */
