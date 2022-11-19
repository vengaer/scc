#include <scc/scc_algorithm.h>

#include <limits.h>

#define SIZE_MASK ((~((size_t)0u)) >> 1u)
#define SIZE_SHIFT ((sizeof(size_t) * CHAR_BIT) - 1u)

size_t scc_algo_lower_bound(void const *key, void const *base, size_t nmemb, size_t size, int(*compare)(void const *, void const *)) {
    return scc_algo_lower_bound_eq(key, base, nmemb, size, compare) & SIZE_MASK;
}

size_t scc_algo_lower_bound_eq(void const *key, void const *base, size_t nmemb, size_t size, int(*compare)(void const *, void const *)) {
    size_t begin = 0u;
    size_t end = nmemb;
    size_t middle;
    int cmp = 0;
    while(begin != end) {
        middle = begin + ((end - begin) >> 1u);
        cmp = compare(key, (unsigned char const *)base + middle * size);
        if(cmp <= 0) {
            end = middle;
        }
        else if(cmp > 0) {
            begin = middle + 1u;
        }
    }

    return begin | (((size_t)!cmp) << SIZE_SHIFT);
}
