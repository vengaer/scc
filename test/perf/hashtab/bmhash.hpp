#ifndef BMHASH_HPP
#define BMHASH_HPP

#include <cstddef>

template <typename T>
struct fnv1a {
    std::size_t operator()(T const& val) const noexcept {
        unsigned long long constexpr scc_fnv_offset_basis =
            0xcbf29ce484222325ull;
        unsigned long long constexpr scc_fnv_prime =
            0x100000001b3ull;

        unsigned long long hash = scc_fnv_offset_basis;
        unsigned char const *data =
            reinterpret_cast<unsigned char const *>(&val);

        for(size_t i = 0; i < sizeof(val); ++i) {
            hash ^= data[i];
            hash *= scc_fnv_prime;
        }
        return hash;
    }
};

#endif /* BMHASH_HPP */
