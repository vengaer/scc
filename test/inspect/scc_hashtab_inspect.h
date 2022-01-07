#ifndef SCC_HASHTAB_INSPECT_H
#define SCC_HASHTAB_INSPECT_H

#include <scc/scc_hashtab.h>

struct scc_hashtab *scc_hashtab_inspect_base(void *tab);
scc_hashtab_metatype *scc_hashtab_inspect_md(void *tab);

#endif /* SCC_HASHTAB_INSPECT_H */
