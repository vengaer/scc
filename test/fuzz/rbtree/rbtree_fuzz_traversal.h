#ifndef RBTREE_FUZZ_TRAVERSAL_H
#define RBTREE_FUZZ_TRAVERSAL_H

#include <scc/scc_rbtree.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool rbtree_fuzz_traversal(scc_rbtree(uint32_t) const *handle, uint32_t const *data, size_t size);

#endif /* RBTREE_FUZZ_TRAVERSAL_H */
