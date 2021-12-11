#ifndef RBTREE_FUZZ_REMOVAL_H
#define RBTREE_FUZZ_REMOVAL_H

#include <scc/scc_rbtree.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool rbtree_fuzz_removal(scc_rbtree(uint32_t) *handle, uint32_t *data, size_t size);

#endif /* RBTREE_FUZZ_REMOVAL_H */
