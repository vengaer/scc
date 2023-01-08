#ifndef SCC_HASHMAP_INSPECT_H
#define SCC_HASHMAP_INSPECT_H

#include <scc/hashmap.h>

scc_hashmap_metatype *scc_hashmap_inspect_metadata(void *map);
struct scc_hashmap_base *scc_hashmap_inspect_base(void *map);

#ifdef SCC_CANARY_ENABLED
unsigned char const *scc_hashmap_inspect_canary(void *map);
#endif

#endif /* SCC_HASHMAP_INSPECT_H */
