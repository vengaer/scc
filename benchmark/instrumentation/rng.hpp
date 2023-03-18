#ifndef RNG_HPP
#define RNG_HPP

#include <instrumentation/types.h>
#include <benchmark/benchmark.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

namespace rng {

template <typename T = bm_type>
std::vector<T> shuffled_iota(benchmark::State const& state) {
    std::vector<T> data(state.range(0));

    std::iota(std::begin(data), std::end(data), T{});
    std::mt19937 rng{std::random_device{}()};
    std::shuffle(std::begin(data), std::end(data), rng);

    return data;
}

}

#endif /* RNG_HPP */
