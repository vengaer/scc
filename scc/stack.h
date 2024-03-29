#ifndef SCC_STACK_H
#define SCC_STACK_H

#include "pp_token.h"
#include "vec.h"

#ifndef SCC_STACK_CONTAINER
//! .. c:macro:: SCC_STACK_CONTAINER
//!
//!     Macro expanding to the underlying container used by the stack.
//!     May be defined by user-code before including the header, thereby
//!     allowing for using a different container.
//!
//!     ..
//!       _TODO - custome container interface
//!
//!     Unless overridden, the underlying type is an :ref:`scc_vec <scc_vec>`.
#define SCC_STACK_CONTAINER scc_vec
#endif

//! .. c:macro:: scc_stack(type)
//!
//!     Expands to a type suitable for referring to a stack holding
//!     instances of the given :c:texpr:`type`.
//!
//!     :param type: The type to store in the stack
#define scc_stack(type) scc_pp_expand(SCC_STACK_CONTAINER)(type)

//! .. _scc_stack_new:
//! .. c:function:: void *scc_stack_new(type)
//!
//!     Initializes a stack holding instances of the given :c:texpr:`type`.
//!     The lifetime of the stack is determined by the underlying container.
//!
//!     .. seealso::
//!
//!         :ref:`scc_stack_new_dyn <scc_stack_new_dyn>` for a dynamically
//!         allocated stack
//!
//!     :param type: The type to be stored in the stack
//!     :returns: A handle to the underlying container
#define scc_stack_new(type) scc_pp_cat_expand(SCC_STACK_CONTAINER,_new)(type)

//! .. _scc_stack_new_dyn:
//! .. c:function:: void *scc_stack_new_dyn(type)
//!
//!     Like :ref:`scc_stack_new <scc_stack_new>` except for the container
//!     being allocated on the heap
//!
//!     .. note::
//!
//!         Unlike ``scc_stack_new``, ``scc_stack_new_dyn`` may fail. The returned
//!         pointer should always be checked against ``NULL``
//!
//!     :param type: The type to be stored in the stack
//!     :returns: A handle to the underlying container, or ``NULL`` on allocation
//!               failure
#define scc_stack_new_dyn(type) scc_pp_cat_expand(SCC_STACK_CONTAINER,_new)(type)

//! .. c:function:: void scc_stack_free(void *stack)
//!
//!     Free any memory used by the stack. May be a nop depending on the
//!     underlying container
//!
//!     :param stack: The stack to free
#define scc_stack_free(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_free)(stack)

//! .. c:function:: _Bool scc_stack_push(void *stackaddr, type value)
//!
//!     Push :c:texpr:`value` to the given stack. The call may result in
//!     :c:texpr:`*(void **)stackaddr` being reallocated should the underlying
//!     container be dynamic.
//!
//!     :param stackaddr: Address of the stack to which the value is to be pushed
//!     :param value: The value to push to the stack
//!     :returns: A :c:texpr:`_Bool` indicating whether the insertion was successful
//!     :retval true: :c:texpr:`value` was successfully pushed to :c:texpr:`*(void **)stackaddr`. :c:texpr:`*(void **)stackaddr`
//!                   may have been reallocated
//!     :retval false: :c:texpr:`value` could not be added to the underlying container
#define scc_stack_push(stackaddr, ...) scc_pp_cat_expand(SCC_STACK_CONTAINER,_push)(stackaddr, (__VA_ARGS__))

#define scc_stack_pop(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_pop)(stack)
#define scc_stack_top(stack) stack[scc_pp_cat_expand(SCC_STACK_CONTAINER,_size)(stack) - 1]
#define scc_stack_size(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_size)(stack)
#define scc_stack_empty(stack) (!scc_stack_size(stack))
#define scc_stack_capacity(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_capacity)(stack)
#define scc_stack_clear(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_clear)(stack)
#define scc_stack_reserve(stack, capacity) scc_pp_cat_expand(SCC_STACK_CONTAINER,_reserve)(stack, capacity)

#endif /* SCC_STACK_H */
