#ifndef SCC_STACK_H
#define SCC_STACK_H

#include "pp_token.h"
#include "vec.h"

#ifndef SCC_STACK_CONTAINER
#define SCC_STACK_CONTAINER scc_vec
#endif

/**
 * Expands to an opaque pointer suitable for referring to a stack containing elements of \a type
 *
 * \param type The type to store in the stack
 */
#define scc_stack(type) scc_pp_expand(SCC_STACK_CONTAINER)(type)

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_stack_new:
 * \endverbatim
 *
 * Initialize a stack storing instances of the specified \a type.
 *
 * The resulting stack is small-size optimized, meaning it is placed
 * in the stack frame in which the macro is invoked. Subsequent operations
 * may cause it to be moved to the heap.
 *
 * Regardless of size, users are responsible for destroying the
 * vector using @verbatim embed:rst:inline :ref:`scc_stack_free <scc_stack_free>` @endverbatim.
 *
 * \sa @verbatim embed:rst:inline :ref:`scc_stack_new_dyn <scc_stack_new_dyn>` @endverbatim
 *     when requiring dynamically allocated instances.
 *
 * \param type The type to store in the stack.
 *
 * \return A handle to an instantiated stack storing \a type instances.
 */
#define scc_stack_new(type) scc_pp_cat_expand(SCC_STACK_CONTAINER,_new)(type)

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_stack_new_dyn:
 * \endverbatim
 *
 * Like @verbatim embed:rst:inline :ref:`scc_stack_new <scc_stack_new>` @endverbatim
 * except that the instance is allocated on the heap rather than on the stack.
 *
 * \note Unlike @verbatim embed:rst:inline :ref:`scc_stack_new <scc_stack_new>` @endverbatim,
 *       ``scc_stack_new_dyn`` may fail. If it does, ``NULL`` is returned.
 *
 * \param type The type to store in the stack.
 *
 * \return A handle to a stack storing \a type instances, or ``NULL`` on failure.
 */
#define scc_stack_new_dyn(type) scc_pp_cat_expand(SCC_STACK_CONTAINER,_new)(type)

/**
 * \verbatim embed:rst:leading-asterisk
 *  .. _scc_stack_free:
 * \endverbatim
 *
 * Reclaim memory occupied by the supplied stack
 *
 * \param stack Handle identifying that stack to free
 */
#define scc_stack_free(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_free)(stack)

/**
 * Push element to the top of the stack
 *
 * \note ``scc_stack_push`` takes a \b pointer to the handle returned by one of the
 * @verbatim embed:rst:inline :ref:`initialization constructs <stack_init>` @endverbatim,
 * \b not the handle itself.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/stack/push_pop.c
 *      :caption: Pushing values to and poping values from a stack
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \note The value parameter is implemented using \a ... to allow the passing of compound literals.
 *
 * \param stackaddr Address of the handle identifying the stack
 * \param ... Element to insert in the stack
 *
 * \return ``true`` if the element could be inserted, otherwise ``false``
 */
#define scc_stack_push(stackaddr, ...) scc_pp_cat_expand(SCC_STACK_CONTAINER,_push)(stackaddr, (__VA_ARGS__))

/**
 * Pop and return topmost value from the stack.
 *
 * \warning Invoking this on an empty instance is a \b very \b bad idea
 *
 * \param stack The instance to pop from
 *
 * \return The value that was topmost in the stack at the time of the call
 */
#define scc_stack_pop(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_pop)(stack)

/**
 * Access the topmost value in the stack
 *
 * \warning Invoking this on an empty instance is a \b pretty \b bad idea
 *
 * \param stack The instance to pop from
 *
 * \return The value that was topmost in the stack at the time of the call
 */
#define scc_stack_top(stack) stack[scc_pp_cat_expand(SCC_STACK_CONTAINER,_size)(stack) - 1]

/**
 * Query the size of the stack
 *
 * \param Handle identifying the stack instance
 *
 * \return Number of elements stored in the stack
 */
#define scc_stack_size(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_size)(stack)

/**
 * Determine wheter the stack is empty of not
 *
 * \param Handle identifying the stack instance
 *
 * \return ``true`` if the stack is empty, otherwise ``false``
 */
#define scc_stack_empty(stack) (!scc_stack_size(stack))

/**
 * Determine the current capacity of the stack
 *
 * \note Should the capacity be exhausted, the stack is automatically reallocated
 *
 * \param stack Handle identifying the stack instance
 *
 * \return The maximum number of elements the stack is currently able to store
 */
#define scc_stack_capacity(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_capacity)(stack)

/**
 * Remove all the elements in the stack
 *
 * \param stack Handle identifying the stack instance
 */
#define scc_stack_clear(stack) scc_pp_cat_expand(SCC_STACK_CONTAINER,_clear)(stack)

/**
 * Reserve enough memory to store at least \a capacity elements
 *
 * \param stack Handle identifying the stack instance
 * \param capacity The desired capacity
 *
 * \return ``true`` on success, otherwise ``false``
 */
#define scc_stack_reserve(stack, capacity) scc_pp_cat_expand(SCC_STACK_CONTAINER,_reserve)(stack, capacity)

#endif /* SCC_STACK_H */
