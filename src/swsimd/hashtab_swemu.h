#ifndef HASHTAB_SWEMU_H
#define HASHTAB_SWEMU_H

#include <stddef.h>

struct scc_hashtab_base;

long long scc_hashtab_probe_insert(struct scc_hashtab_base const *base, void const *handle, size_t elemsize, unsigned long long hash);
//? .. c:function:: long long scc_hashtab_probe_find(struct scc_hashtab_base const *base, \
//?     void const *handle, size_t elemsize, unsigned long long hash)
//?
//?     Software-emulated SIMD find probing, looking for the element at ``handle``
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
long long scc_hashtab_probe_find(struct scc_hashtab_base const *base, void const *handle, size_t elemsize, unsigned long long hash);

#endif /* HASHTAB_SWEMU_H */
