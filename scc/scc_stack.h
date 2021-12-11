#ifndef SCC_STACK_H
#define SCC_STACK_H

#include "scc_pp_token.h"
#include "scc_svec.h"

#ifndef SCC_STACK_CONTAINER
#define SCC_STACK_CONTAINER scc_svec
#endif

#define scc_stack(type) scc_pp_expand(SCC_STACK_CONTAINER)(type)

#define scc_stack_init(type) scc_pp_cat_expand(SCC_STACK_CONTAINER,_init)(type)
#define scc_stack_free(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_free)(stack)
#define scc_stack_push(stack, value) scc_pp_cat_expand(SCC_STACK_CONTAINER,_push)(stack, value)
#define scc_stack_pop(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_pop)(stack)
#define scc_stack_top(stack) stack[scc_pp_cat_expand(SCC_STACK_CONTAINER,_size)(stack) - 1]
#define scc_stack_size(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_size)(stack)
#define scc_stack_capacity(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_capacity)(stack)
#define scc_stack_clear(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_clear)(stack)
#define scc_stack_reserve(stack, capacity) scc_pp_cat_expand(SCC_STACK_CONTAINER,_reserve)(stack, capacity)

#endif /* SCC_STACK_H */
