#ifndef FIND_HPP
#define FIND_HPP

#include <benchmark/benchmark.h>

void btree_find_setup(benchmark::State const& state);
void btree_find_teardown(benchmark::State const& state) noexcept;
void btree_find(benchmark::State& state);

#endif /* FIND_HPP */
