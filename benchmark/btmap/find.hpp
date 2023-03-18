#ifndef FIND_HPP
#define FIND_HPP

#include <benchmark/benchmark.h>

void btmap_find_setup(benchmark::State const& state);
void btmap_find_teardown(benchmark::State const& state) noexcept;
void btmap_find(benchmark::State& state);

#endif /* FIND_HPP */
