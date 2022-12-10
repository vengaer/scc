#ifndef SCC_RBTREE_INSPECT_H
#define SCC_RBTREE_INSPECT_H

#include <scc/rbtree.h>

#include <limits.h>

#ifndef SCC_TYPE_INSPECT_MASK
#define SCC_TYPE_INSPECT_MASK
typedef unsigned scc_inspect_mask;
#endif /* SCC_TYPE_INSPECT_MASK */

#define SCC_RBTREE_ERR_RED   0x01
#define SCC_RBTREE_ERR_BLACK 0x02
#define SCC_RBTREE_ERR_LEFT  0x04
#define SCC_RBTREE_ERR_RIGHT 0x08
#define SCC_RBTREE_ERR_ROOT  0x10
#define SCC_RBTREE_ERR_LOOP  0x20

#define SCC_RBTREE_ERR_MASK (SCC_RBTREE_ERR_RED   | \
                             SCC_RBTREE_ERR_BLACK | \
                             SCC_RBTREE_ERR_LEFT  | \
                             SCC_RBTREE_ERR_RIGHT | \
                             SCC_RBTREE_ERR_ROOT  | \
                             SCC_RBTREE_ERR_LOOP)

scc_inspect_mask scc_rbtree_inspect_properties(void const *handle);
void scc_rbtree_inspect_dump_flags(scc_inspect_mask flags);

#endif /* SCC_RBTREE_INSPECT_H */
