#include "scc_hashmap_inspect.h"

struct scc_hashmap_base *scc_hashmap_inspect_base(void *map) {
    struct scc_hashmap_base *base = scc_container(
        (unsigned char *)map - ((unsigned char *)map)[-1] - sizeof(base->hm_fwoff),
        struct scc_hashmap_base,
        hm_fwoff
    );
    return base;
}
