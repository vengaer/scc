#ifndef HASHMAP_SWEMU_H
#define HASHMAP_SWEMU_H

#include <stddef.h>

struct scc_hashmap_base;

//? .. c:function:: unsigned long long scc_hashmap_probe_insert(struct scc_hashmap_base const *base, \
//?     void const *handle, size_t elemsize, unsigned long long hash)
//?
//?     Software-emulated SIMD insert probing, computing the slot to insert the key
//?     at ``handle`` in
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash map
//?     :param handle: Hash map handle
//?     :param keysize: Size of the keys in the hash map
//?     :param hash: Hash of the key to be inserted
//?     :returns: Index of the slot to insert the element in with the MSB
//?               set iff the key to insert was already present in the map
unsigned long long scc_hashmap_probe_insert(struct scc_hashmap_base const *base, void const *handle, size_t keysize, unsigned long long hash);

//? .. c:function:: long long scc_hashmap_probe_find(struct scc_hashmap_base const *base, \
//?     void const *handle, size_t elemsize, unsigned long long hash)
//?
//?     Software-emulated SIMD find probing, looking for the key at ``handle``
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base: Base address of the hash map
//?     :param handle: Hash map handle
//?     :param keysize: Size of the keys in the hash map
//?     :param hash: Hash of the key to be found
//?     :returns: Index of the slot containing the matching value,
//?               or -1 if no such element is found
long long scc_hashmap_probe_find(struct scc_hashmap_base const *base, void const *handle, size_t keysize, unsigned long long hash);

#endif /* HASHMAP_SWEMU_H */
