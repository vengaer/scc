#ifndef RNG_HPP
#define RNG_HPP

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>

namespace rng {
template <typename T = bm_type>
std::vector<T> iota_shuffle(benchmark::State const& state) {
    std::vector<T> bmdata(state.range(0));
    assert(bmdata.size() == state.range(0));

    std::iota(std::begin(bmdata), std::end(bmdata), bm_type{});

    std::mt19937 rng{std::random_device{}()};
    std::shuffle(std::begin(bmdata), std::end(bmdata), rng);

    return bmdata;
}
} /* namespace rng */

#endif /* RNG_HPP */
