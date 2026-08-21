#ifndef SCC_ARCH_H
#define SCC_ARCH_H

#include "config.h"
#include "pp_token.h"

#include <stddef.h>


#ifdef SCC_HWVEC_SIZE
#define SCC_VECSIZE SCC_HWVEC_SIZE
#else
#define SCC_VECSIZE SCC_SWARVEC_SIZE
#endif

#ifdef SCC_SIMD_ISA
#define scc_arch_select(func)       \
    scc_pp_cat_expand(              \
        scc_pp_cat_expand(          \
            scc_pp_cat_expand(      \
                scc_pp_cat_expand(  \
                    func,_          \
                ),SCC_SIMD_ISA      \
            ),_                     \
        ),trampoline                \
    )

#else
#define scc_arch_select(func)   \
    scc_pp_cat_expand(scc_pp_cat_expand(func,_),swar)
#endif

struct scc_hashmap_base;
struct scc_hashtab_base;

extern unsigned long long scc_arch_select(scc_hashmap_impl_probe_insert)(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
);

extern long long scc_arch_select(scc_hashmap_impl_probe_find)(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
);

extern unsigned long long scc_arch_select(scc_hashtab_impl_probe_insert)(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
);

extern long long scc_arch_select(scc_hashtab_impl_probe_find)(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
);

inline unsigned long long scc_hashmap_impl_probe_insert(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashmap_impl_probe_insert)(base, map, keysize, hash);
}

inline long long scc_hashmap_impl_probe_find(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashmap_impl_probe_find)(base, map, keysize, hash);
}

inline long long scc_hashtab_impl_probe_insert(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashtab_impl_probe_insert)(base, tab, elemsize, hash);
}

inline long long scc_hashtab_impl_probe_find(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashtab_impl_probe_find)(base, tab, elemsize, hash);
}

#endif /* SCC_ARCH_H */
