#ifndef SCC_BTMAP_INSPECT_H
#define SCC_BTMAP_INSPECT_H

#include <stddef.h>
#include <stdio.h>

#ifndef SCC_TYPE_INSPECT_MASK
#define SCC_TYPE_INSPECT_MASK
typedef unsigned scc_inspect_mask;
#endif /* SCC_TYPE_INSPECT_MASK */

#define SCC_BTMAP_ERR_LEFT      0x01
#define SCC_BTMAP_ERR_RIGHT     0x02
#define SCC_BTMAP_ERR_CHILDREN  0x04
#define SCC_BTMAP_ERR_LEAFDEPTH 0x08

scc_inspect_mask scc_btmap_impl_inspect_invariants(void const *btmap, size_t keysize);

#define scc_btmap_inspect_invariants(btmap)         \
    scc_btmap_impl_inspect_invariants((btmap), sizeof((btmap)->btm_key))

void scc_btmap_impl_inspect_dump(void const *restrict btmap, size_t keysize, FILE *fp);

#define scc_btmap_inspect_dump(btmap, fp)           \
    scc_btmap_impl_inspect_dump(btmap, sizeof((btmap)->btm_key), fp)

size_t scc_btmap_inspect_size(void const *btmap);

size_t scc_btmap_impl_inspect_cardinality(void const *restrict btmap, size_t keysize);

#define scc_btmap_inspect_cardinality(btmap, key)  \
    scc_btmap_impl_inspect_cardinality((*(btmap) = (key), btmap), sizeof((btmap)->btm_key))

#endif /* SCC_BTMAP_INSPECT_H */
