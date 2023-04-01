#ifndef ARCH_H
#define ARCH_H

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

//? .. c:function:: unsigned long long scc_hashmap_impl_probe_insert(struct scc_hashmap_base const *base, \
//?     void const *map, size_t elemsize, unsigned long long hash)
//?
//?     Architecture-dependent insertion probing, computing the slot to insert the
//?     key at ``map`` in.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash map
//?     :param map: Hash map handle
//?     :param keysize: Size of the keys in the hash map
//?     :param hash: Hash of the key to be inserted
//?     :returns: Index of the slot to insert the element in with the MSB
//?               set iff the key to insert was already present in the map
inline unsigned long long scc_hashmap_impl_probe_insert(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashmap_impl_probe_insert)(base, map, keysize, hash);
}

//? .. c:function:: long long scc_hashmap_impl_probe_find(struct scc_hashmap_base const *base, \
//?     void const *map, size_t elemsize, unsigned long long hash)
//?
//?     Architecture-specific find probing, looking for the key at ``map``.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash map
//?     :param map: Hash map handle
//?     :param keysize: Size of the keys in the hash map
//?     :param hash: Hash of the key to be found
//?     :returns: Index of the slot containing the matching value,
//?               or -1 if no such element is found
inline long long scc_hashmap_impl_probe_find(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashmap_impl_probe_find)(base, map, keysize, hash);
}

//? .. c:function:: long long scc_hashtab_probe_insert(struct scc_hashtab_base const *base, \
//?     void const *tab, size_t elemsize, unsigned long long hash)
//?
//?     Architecture-specific insert probing, computing the slot to insert the element
//?     at ``tab`` in.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash table
//?     :param tab: Hash tab handle
//?     :param elemsize: Size of the elements in the hash table
//?     :param hash: Hash of the element to be inserted
//?     :returns: Index of the slot to insert the element in, or -1
//?               if the element is already present
inline long long scc_hashtab_impl_probe_insert(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashtab_impl_probe_insert)(base, tab, elemsize, hash);
}

//? .. c:function:: long long scc_hashtab_probe_find(struct scc_hashtab_base const *base, \
//?     void const *tab, size_t elemsize, unsigned long long hash)
//?
//?     Architecture-specific find probing, looking for the element at ``tab``.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash table
//?     :param tab: Hash tab handle
//?     :param elemsize: Size of the element in the hash table
//?     :param hash: Hash of the element to be found
//?     :returns: Index of the slot containing the matching value,
//?               or -1 if no such element is found
inline long long scc_hashtab_impl_probe_find(
    struct scc_hashtab_base const *base,
    void const *tab,
    size_t elemsize,
    unsigned long long hash
) {
    return scc_arch_select(scc_hashtab_impl_probe_find)(base, tab, elemsize, hash);
}

#endif /* ARCH_H */
