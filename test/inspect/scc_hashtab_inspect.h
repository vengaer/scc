#ifndef SCC_HASHTAB_INSPECT_H
#define SCC_HASHTAB_INSPECT_H

#include <scc/scc_hashtab.h>

#define SCC_HASHTAB_INSPECT_VACANT   ((scc_hashtab_metatype)0x0000)
#define SCC_HASHTAB_INSPECT_OCCUPIED ((scc_hashtab_metatype)0x8000)
#define SCC_HASHTAB_INSPECT_GUARD    ((scc_hashtab_metatype)0x4000)
#define SCC_HASHTAB_INSPECT_VACATED  ((scc_hashtab_metatype)0xc000)

struct scc_hashtab_base *scc_hashtab_inspect_base(void *tab);
scc_hashtab_metatype *scc_hashtab_inspect_md(void *tab);

void *scc_hashtab_inspect_impl_data(void *tab, size_t elemsize);

#define scc_hashtab_inspect_data(tab)   \
    scc_hashtab_inspect_impl_data(tab, sizeof(*tab))

#endif /* SCC_HASHTAB_INSPECT_H */
