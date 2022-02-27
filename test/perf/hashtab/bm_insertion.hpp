#ifndef BM_INSERTION_HPP
#define BM_INSERTION_HPP

#include "bmtypes.h"

#include <benchmark/benchmark.h>

void BM_insertion(benchmark::State& state);
void BM_negative_insertion(benchmark::State& state);
void BM_insertion_setup(benchmark::State const& state);
void BM_insertion_teardown(benchmark::State const& state);

void BM_insertion_std(benchmark::State& state);
void BM_negative_insertion_std(benchmark::State& state);
void BM_insertion_std_setup(benchmark::State const& state);
void BM_insertion_std_teardown(benchmark::State const& state);

#endif /* BM_INSERTION_HPP */
