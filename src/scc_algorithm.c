#include <scc/scc_algorithm.h>

size_t scc_algo_lower_bound(void const *key, void const *base, size_t nmemb, size_t size, int(*compare)(void const *, void const *)) {
    size_t begin = 0u;
    size_t end = nmemb;
    size_t middle;
    int cmp;
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

    return begin;
}
