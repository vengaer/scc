#ifndef SCC_STACH_H
#define SCC_STACH_H

#include "scc_svec.h"

#define scc_stack(type) scc_svec(type)

#define scc_stack_init(type) scc_svec_init(type)
#define scc_stack_free(stack) scc_svec_free(stack)
#define scc_stack_push(stack, value) scc_svec_push(stack, value)
#define scc_stack_pop(stack) scc_svec_pop(stack)
#define scc_stack_top(stack) stack[scc_svec_size(stack) - 1]
#define scc_stack_size(stack) scc_svec_size(stack)
#define scc_stack_capacity(stack) scc_svec_capacity(stack)
#define scc_stack_clear(stack) scc_svec_clear(stack)
#define scc_stack_reserve(stack, capacity) scc_svec_reserve(stack, capacity)

#endif /* SCC_STACH_H */
