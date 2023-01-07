#ifndef SCC_HASHTAB_INSPECT_H
#define SCC_HASHTAB_INSPECT_H

#include <scc/hashtab.h>

struct scc_hashtab_base *scc_hashtab_inspect_base(void *tab);
scc_hashtab_metatype *scc_hashtab_inspect_metadata(void *tab);

void *scc_hashtab_inspect_impl_data(void *tab, size_t elemsize);

#define scc_hashtab_inspect_data(tab)   \
    scc_hashtab_inspect_impl_data(tab, sizeof(*(tab)))

#ifdef SCC_CANARY_ENABLED
unsigned char const *scc_hashtab_inspect_canary(void *tab);
#endif

#endif /* SCC_HASHTAB_INSPECT_H */
