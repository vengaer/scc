#ifndef ARCH_H
#define ARCH_H

#include <stddef.h>

struct scc_hashmap_base;
struct scc_hashtab_base;

//? .. c:function:: unsigned long long scc_hashmap_probe_insert(struct scc_hashmap_base const *base, \
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
extern unsigned long long scc_hashmap_probe_insert(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
);

//? .. c:function:: long long scc_hashmap_probe_find(struct scc_hashmap_base const *base, \
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
extern long long scc_hashmap_probe_find(
    struct scc_hashmap_base const *base,
    void const *map,
    size_t keysize,
    unsigned long long hash
);

//? .. c:function:: long long scc_hashtab_probe_insert(struct scc_hashtab_base const *base, \
//?     void const *handle, size_t elemsize, unsigned long long hash)
//?
//?     Architecture-specific insert probing, computing the slot to insert the element
//?     at ``handle`` in.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash table
//?     :param handle: Hash tab handle
//?     :param elemsize: Size of the elements in the hash table
//?     :param hash: Hash of the element to be inserted
//?     :returns: Index of the slot to insert the element in, or -1
//?               if the element is already present
extern long long scc_hashtab_probe_insert(
    struct scc_hashtab_base const *base,
    void const *handle,
    size_t elemsize,
    unsigned long long hash
);

//? .. c:function:: long long scc_hashtab_probe_find(struct scc_hashtab_base const *base, \
//?     void const *handle, size_t elemsize, unsigned long long hash)
//?
//?     Architecture-specific find probing, looking for the element at ``handle``.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash table
//?     :param handle: Hash tab handle
//?     :param elemsize: Size of the element in the hash table
//?     :param hash: Hash of the element to be found
//?     :returns: Index of the slot containing the matching value,
//?               or -1 if no such element is found
extern long long scc_hashtab_probe_find(
    struct scc_hashtab_base const *base,
    void const *handle,
    size_t elemsize,
    unsigned long long hash
);

#endif /* ARCH_H */
