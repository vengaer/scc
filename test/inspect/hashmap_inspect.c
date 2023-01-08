#include "hashmap_inspect.h"

scc_hashmap_metatype *scc_hashmap_inspect_metadata(void *map) {
    struct scc_hashmap_base *base = scc_hashmap_inspect_base(map);
    return (void *)((unsigned char *)base + base->hm_mdoff);
}

struct scc_hashmap_base *scc_hashmap_inspect_base(void *map) {
    struct scc_hashmap_base *base = scc_container(
        (unsigned char *)map - ((unsigned char *)map)[-1] - sizeof(base->hm_fwoff),
        struct scc_hashmap_base,
        hm_fwoff
    );
    return base;
}

#ifdef SCC_CANARY_ENABLED
unsigned char const *scc_hashmap_inspect_canary(void *map) {
    struct scc_hashmap_base *base = scc_hashmap_inspect_base(map);
    return (unsigned char *)scc_hashmap_inspect_metadata(map) +
            (base->hm_capacity + SCC_HASHMAP_GUARDSZ) * sizeof(scc_hashmap_metatype);
}
#endif
