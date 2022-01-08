#ifndef SCC_HASHTAB_INSPECT_H
#define SCC_HASHTAB_INSPECT_H

#include <scc/scc_hashtab.h>

#define SCC_HASHTAB_INSPECT_VACANT   ((scc_hashtab_metatype)0x0000)
#define SCC_HASHTAB_INSPECT_OCCUPIED ((scc_hashtab_metatype)0x8000)
#define SCC_HASHTAB_INSPECT_GUARD    ((scc_hashtab_metatype)0x4000)
#define SCC_HASHTAB_INSPECT_VACATED  ((scc_hashtab_metatype)0xc000)

struct scc_hashtab *scc_hashtab_inspect_base(void *tab);
scc_hashtab_metatype *scc_hashtab_inspect_md(void *tab);

#endif /* SCC_HASHTAB_INSPECT_H */
