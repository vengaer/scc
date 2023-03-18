#ifndef INSERT_HPP
#define INSERT_HPP

#include <benchmark/benchmark.h>

void btree_insert_setup(benchmark::State const& state) noexcept;
void btree_insert_teardown(benchmark::State const& state) noexcept;
void btree_insert(benchmark::State& state);

#endif /* INSERT_HPP */
