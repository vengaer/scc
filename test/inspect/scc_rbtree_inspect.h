#ifndef SCC_RBTREE_INSPECT_H
#define SCC_RBTREE_INSPECT_H

#include <scc/scc_rbtree.h>

#include <limits.h>

#define SCC_RBTREE_ERR_RED   (1ull << (sizeof(0ull) * CHAR_BIT - 1u))
#define SCC_RBTREE_ERR_BLACK (1ull << (sizeof(0ull) * CHAR_BIT - 2u))
#define SCC_RBTREE_ERR_LEFT  (1ull << (sizeof(0ull) * CHAR_BIT - 3u))
#define SCC_RBTREE_ERR_RIGHT (1ull << (sizeof(0ull) * CHAR_BIT - 4u))
#define SCC_RBTREE_ERR_ROOT  (1ull << (sizeof(0ull) * CHAR_BIT - 5u))

#define SCC_RBTREE_ERR_MASK (SCC_RBTREE_ERR_RED   | \
                             SCC_RBTREE_ERR_BLACK | \
                             SCC_RBTREE_ERR_LEFT  | \
                             SCC_RBTREE_ERR_RIGHT | \
                             SCC_RBTREE_ERR_ROOT)

unsigned long long scc_rbtree_inspect_properties(void const *handle);
void scc_rbtree_inspect_dump_flags(unsigned long long flags);

#endif /* SCC_RBTREE_INSPECT_H */
