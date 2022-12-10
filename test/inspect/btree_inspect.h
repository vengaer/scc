#ifndef SCC_BTREE_INSPECT_H
#define SCC_BTREE_INSPECT_H

#include <stddef.h>
#include <stdio.h>

#ifndef SCC_TYPE_INSPECT_MASK
#define SCC_TYPE_INSPECT_MASK
typedef unsigned scc_inspect_mask;
#endif /* SCC_TYPE_INSPECT_MASK */

#define SCC_BTREE_ERR_LEFT      0x01
#define SCC_BTREE_ERR_RIGHT     0x02
#define SCC_BTREE_ERR_CHILDREN  0x04
#define SCC_BTREE_ERR_LEAFDEPTH 0x08

scc_inspect_mask scc_btree_impl_inspect_invariants(void const *btree, size_t elemsize);

#define scc_btree_inspect_invariants(btree)         \
    scc_btree_impl_inspect_invariants((btree), sizeof(*(btree)))

void scc_btree_impl_inspect_dump(void const *restrict btree, size_t elemsize, FILE *fp);

#define scc_btree_inspect_dump(btree, fp)           \
    scc_btree_impl_inspect_dump(btree, sizeof(*(btree)), fp)

size_t scc_btree_inspect_size(void const *btree);

size_t scc_btree_impl_inspect_cardinality(void const *restrict btree, size_t elemsize);

#define scc_btree_inspect_cardinality(btree, elem)  \
    scc_btree_impl_inspect_cardinality((*(btree) = (elem), btree), sizeof(*(btree)))

#endif /* SCC_BTREE_INSPECT_H */
