#ifndef SCC_HASHTAB_INSPECT_H
#define SCC_HASHTAB_INSPECT_H

#include <scc/scc_hashtab.h>

struct scc_hashtab_base *scc_hashtab_inspect_base(void *tab);
scc_hashtab_metatype *scc_hashtab_inspect_metadata(void *tab);

void *scc_hashtab_inspect_impl_data(void *tab, size_t elemsize);

#define scc_hashtab_inspect_data(tab)   \
    scc_hashtab_inspect_impl_data(tab, sizeof(*(tab)))

#endif /* SCC_HASHTAB_INSPECT_H */
