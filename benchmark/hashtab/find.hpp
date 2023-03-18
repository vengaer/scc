#ifndef FIND_HPP
#define FIND_HPP

#include <benchmark/benchmark.h>

void hashtab_find_setup(benchmark::State const& state);
void hashtab_find_teardown(benchmark::State const& state) noexcept;
void hashtab_find(benchmark::State& state);

#endif /* FIND_HPP */
