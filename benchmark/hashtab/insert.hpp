#ifndef INSERT_HPP
#define INSERT_HPP

#include <benchmark/benchmark.h>

void hashtab_insert_setup(benchmark::State const& state) noexcept;
void hashtab_insert_teardown(benchmark::State const& state) noexcept;
void hashtab_insert(benchmark::State& state);

#endif /* INSERT_HPP */
