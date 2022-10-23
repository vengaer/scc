#ifndef RBTREE_FUZZ_TRAVERSAL_H
#define RBTREE_FUZZ_TRAVERSAL_H

#include <scc/scc_rbtree.h>

#include <stddef.h>
#include <stdint.h>

void rbtree_fuzz_traversal(scc_rbtree(uint32_t) const handle, uint32_t const *data, size_t size);  /* NOLINT(readability-avoid-const-params-in-decls) */

#endif /* RBTREE_FUZZ_TRAVERSAL_H */
