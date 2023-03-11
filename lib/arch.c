#include <scc/arch.h>

unsigned long long scc_hashmap_impl_probe_insert(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
);

long long scc_hashmap_impl_probe_find(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
);

long long scc_hashtab_impl_probe_insert(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
);

long long scc_hashtab_impl_probe_find(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
);
