#ifndef HASHTAB_SWEMU_H
#define HASHTAB_SWEMU_H

#include <stddef.h>

struct scc_hashtab_base;

long long scc_hashtab_probe_insert(struct scc_hashtab_base const *base, void const *handle, size_t elemsize, unsigned long long hash);
long long scc_hashtab_probe_find(struct scc_hashtab_base const *base, void const *handle, size_t elemsize, unsigned long long hash);

#endif /* HASHTAB_SWEMU_H */
