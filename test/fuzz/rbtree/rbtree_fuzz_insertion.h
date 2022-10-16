#ifndef RBTREE_FUZZ_INSERTION_H
#define RBTREE_FUZZ_INSERTION_H

#include <scc/scc_rbtree.h>

#include <stddef.h>
#include <stdint.h>

void rbtree_fuzz_insert(scc_rbtree(uint32_t) *handle, uint32_t *data, unsigned unique_end, size_t size);

#endif /* RBTREE_FUZZ_INSERTION_H */
