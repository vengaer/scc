#ifndef SCC_ALGORITHM_H
#define SCC_ALGORITHM_H

#include <stddef.h>

_Bool scc_algo_impl_lower_bound_is_linear(size_t size);

size_t scc_algo_lower_bound(void const *key, void const *base, size_t nmemb, size_t size, int(*compare)(void const *, void const *));

size_t scc_algo_lower_bound_eq(void const *key, void const *base, size_t nmemb, size_t size, int(*compare)(void const *, void const *));

#endif /* SCC_ALGORITHM_H */
