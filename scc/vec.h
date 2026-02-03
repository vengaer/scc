#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "bug.h"
#include "mem.h"
#include "pp_token.h"

#include <stddef.h>

/**
 * Capacity of the stack buffer used for small-size
 * optimization.
 *
 * Users may override this value when using the library
 * by providing a preprocessor definition with this name
 * before including the header.
 *
 * \warning Must be greater than 0
 */
#ifndef SCC_VEC_STATIC_CAPACITY
#define SCC_VEC_STATIC_CAPACITY 32
#endif

#if SCC_VEC_STATIC_CAPACITY <= 0
#error Static capacity must be greater than 0
#endif

/**
 * Expands to an opaque pointer suitable for referring to a
 * vector containing instance of the provided \a type.
 *
 * \param type The type to store in the vector.
 *
 * The macro is as the type of a variable or parameter
 * declaration along the lines of
 *
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c
 *      :caption: Creating a ``vec`` for storing ``int`` instances.
 *
 *      scc_vec(int) ivec;
 * \endverbatim
 */
#define scc_vec(type) type *

/**
 * Expands to an opaque pointer suitable for iterating over
 * a vector.
 *
 * \param type The type stored in the ``vec``
 */
#define scc_vec_iter(type) scc_vec(type)

struct scc_vec_base {
    size_t sv_size;
    size_t sv_capacity;
    unsigned char sv_buffer[];
};

#define scc_vec_impl_layout(type)                                       \
    struct {                                                            \
        struct {                                                        \
            size_t sv_size;                                             \
            size_t sv_capacity;                                         \
            unsigned char sv_npad;                                      \
            unsigned char sv_dynalloc;                                  \
        } v0;                                                           \
        type sv_buffer[SCC_VEC_STATIC_CAPACITY];                        \
    }

#define scc_vec_impl_base_qual(vec, qual)                               \
    scc_container_qual(                                                 \
        (unsigned char qual *)(vec) - scc_vec_impl_npad(vec),           \
        struct scc_vec_base,                                            \
        sv_buffer,                                                      \
        qual                                                            \
    )

#define scc_vec_impl_base(vec)                                          \
    scc_vec_impl_base_qual(vec,)

#define scc_vec_impl_offset(type)                                       \
    sizeof(                                                             \
        struct {                                                        \
            struct {                                                    \
                size_t sv_size;                                         \
                size_t sv_capacity;                                     \
                unsigned char sv_npad;                                  \
                unsigned char sv_dynalloc;                              \
            } v0;                                                       \
            type sv_buffer[];                                           \
        }                                                               \
    )

void *scc_vec_impl_new(struct scc_vec_base *base, size_t offset, size_t capacity);

void *scc_vec_impl_new_dyn(size_t vecsz, size_t offset, size_t capacity);

/**
 * Initialize a vector storing instances of the specified \a type.
 *
 * The resulting vector is small-size optimized, meaning it is placed
 * in the stack frame in which the macro is invoked. Subsequent operations
 * may cause it to be moved to the heap.
 *
 * Regardless of size, users are responsible for destroying the
 * vector using ``scc_vec_free``.
 *
 * \param type The type to store in the vector.
 *
 * \return A handle to an instantiated vector storing \a type instances.
 */
#define scc_vec_new(type)                                                \
    (type *)scc_vec_impl_new(                                            \
        (void *)&(scc_vec_impl_layout(type)){ 0 },                       \
        scc_vec_impl_offset(type),                                       \
        SCC_VEC_STATIC_CAPACITY                                          \
    )

/**
 * Initializes a new ``vec`` instance storing instances of \a type.
 *
 * The effects are similar to those of  `scc_vec_new` except that
 * the ``vec`` is placed on the heap from the get-go
 *
 * \note
 *   The macro may fail in which case ``NULL`` is returned. Validating the
 *   return value is highly encouraged.
 *
 *
 * \param type The type to store in the ``vec``.
 *
 * \return A handle to the new ``vec``, or ``NULL`` on failure.
 */
#define scc_vec_new_dyn(type)                                            \
    (type *)scc_vec_impl_new_dyn(                                        \
        sizeof(scc_vec_impl_layout(type)),                               \
        scc_vec_impl_offset(type),                                       \
        SCC_VEC_STATIC_CAPACITY                                          \
    )

void *scc_vec_impl_from(void *restrict vec, void const *restrict data, size_t size, size_t elemsize);

void *scc_vec_impl_from_dyn(size_t basecap, size_t offset, void const *data, size_t size, size_t elemsize);

/**
 * Create and instantiate a ``vec`` holding the provided values, each
 * interpreted as an instance of \a type.
 *
 * The variadic parameters are written to the ``vec`` in the provided order.
 *
 * If the variadic parameters to not all fit in the small-size optimization
 * buffer, the ``vec`` is placed on the heap at instantiation.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/vec/initialize_at_instantiation.c
 *      :caption: Initialize the ``vec`` at instantiation
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param type The type of instances to store in the ``vec``.
 * \param ... Arbitrary number of values to initialize the ``vec`` with.
 *
 * \return An opaque pointer suitable for referring to the ``vec``, or ``NULL`` on faliure.
 */
#define scc_vec_from(type, ...)                                             \
    scc_vec_impl_from(                                                      \
        scc_vec_new(type),                                                  \
        (type[]){ __VA_ARGS__ },                                            \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                             \
        sizeof(type)                                                        \
    )

/**
 * Like ``scc_vec_from`` except that the ``vec`` is guaranteed to be allocated
 * on the heap.
 *
 * \param type The type of instances to store in the ``vec``.
 * \param ... Arbitrary number of values to initialize the ``vec`` with.
 *
 * \return An opaque pointer suitable for referring to the ``vec``, or ``NULL`` on faliure.
 */
#define scc_vec_from_dyn(type, ...)                                         \
    scc_vec_impl_from_dyn(                                                  \
        SCC_VEC_STATIC_CAPACITY,                                            \
        scc_vec_impl_offset(type),                                          \
        (type[]){ __VA_ARGS__ },                                            \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                             \
        sizeof(type)                                                        \
    )

_Bool scc_vec_impl_resize(void *vecaddr, size_t size, size_t elemsize);

/**
 * Resize the given ``vec`` to the specified \a size.
 *
 * The ``vec`` is either truncated or reallocated as required. Newly allocated elements
 * are zeroed. On truncation, elements beyond the new end of the ``vec`` are
 * left as-is.
 *
 * \note As this may potentially reallocate the ``vec``, other references to
 * the instance should be treated as invalid once this has returned.
 *
 * \param vecaddr Address of the handle to the ``vec`` to be resized.
 * \param size    Desired size of the ``vec``.
 *
 * \return ``true`` on success, ``false`` on failure.
 */
#define scc_vec_resize(vecaddr, size)                                       \
    scc_vec_impl_resize(vecaddr, size, sizeof(**(vecaddr)))

void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize);

/**
 * Erase a single element at the provided \a index.
 *
 * Values beyond the \a index are "shifted" down to fill the exposed hole.
 *
 * \param vec Handle to the ``vec`` from which the element is to be removed
 * \param index Index of the element to remove
 */
#define scc_vec_erase(vec, index)                                       \
    scc_vec_impl_erase(vec, index, sizeof(*(vec)))

void scc_vec_impl_erase_range(void *vec, size_t first, size_t end, size_t elemsize);

/**
 * Erase elements in the range [\a first,\a end).
 *
 * Potential elements beyond the erased range are shifted down to replace
 * removed ones.
 *
 * \param vec Handle to the ``vec`` from which elements are to be removed.
 * \param first First elements to be erased
 * \param end Element beyond the last to be removed. C.f. C++'s end iterator.
 */
#define scc_vec_erase_range(vec, first, end)                            \
    scc_vec_impl_erase_range(vec, first, end, sizeof(*(vec)))

_Bool scc_vec_impl_push_ensure_capacity(void *vecaddr, size_t elemsize);

/**
 * Push the given \a value to the back of the ``vec``.
 *
 * The ``vec`` is reallocated as required. Potential other references to the
 * ``vec`` are invalidated.
 *
 * The type of the \a value parameter must be convertible to the type
 * stored in ``vec`` whose address is provided.
 *
 * \param vecaddr Address of the ``vec`` handle.
 * \param value The value to push.
 *
 * \return ``true`` on success, otherwise ``false``:
 */
#define scc_vec_push(vecaddr, value)                                        \
    (scc_vec_impl_push_ensure_capacity(vecaddr, sizeof(**(vecaddr))) &&     \
    ((*(vecaddr))[scc_vec_impl_base(*(vecaddr))->sv_size++] = (value),1))

_Bool scc_vec_impl_reserve(void *vecaddr, size_t capacity, size_t elemsize);

/**
 * Reserve enough memory to store at least \a capacity elements.
 *
 * The ``vec`` is reallocated as required. Potential other references
 * are invalidated.
 *
 * \param vecaddr  Address of the ``vec`` handle.
 * \param capacity The desired capacity
 *
 * \return ``true`` on success, otherwise ``false``.
 */
#define scc_vec_reserve(vecaddr, capacity)                                  \
    scc_vec_impl_reserve((vecaddr), capacity, sizeof(**(vecaddr)))

inline size_t scc_vec_impl_npad(void const *vec) {
    return ((unsigned char const *)vec)[-2] + 2 * sizeof(unsigned char);
}

/**
 * Reclaim memory allocated for the given ``vec``.
 *
 * Used regardless of which method was used to create the
 * ``vec``.
 *
 * \param vec Handle to the ``vec`` to drop
 */
void scc_vec_free(void *vec);

/**
 * Query the size of the provided ``vec``
 *
 * \param vec Handle to the ``vec``.
 *
 * \return The size of the provided ``vec``.
 */
inline size_t scc_vec_size(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_size;
}

/**
 * Query the capacity of the provided ``vec``.
 *
 * \param vec Handle to the ``vec``.
 *
 * \return The capacity of the provided ``vec``.
 */
inline size_t scc_vec_capacity(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_capacity;
}

/**
 * Determine whether or not the provided ``vec`` is empty
 *
 * Equivalent to comparing the size of the ``vec`` to 0.
 *
 * \param vec Handle to the ``vec``.
 *
 * \return ``true`` if the provided ``vec`` is empty, otherwise ``false``:
 */
inline _Bool scc_vec_empty(void const *vec) {
    return !scc_vec_size(vec);
}

/**
 * Pop the last value off the ``vec``.
 *
 * The macro expands to the popped value.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/vec/pop_last.c
 *      :caption: Pop and use the last value in a ``vec``
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 *
 * \param vec Handle to the ``vec``
 */
#define scc_vec_pop(vec)                                                    \
    ((vec)[--scc_vec_impl_base(vec)->sv_size])

/**
 * Clear the contents of the ``vec``.
 *
 * While the size of the ``vec`` is set to 0, the
 * capacity is left unchanged.
 *
 * \param vec Handle to the ``vec``
 */
inline void scc_vec_clear(void *vec) {
    scc_vec_impl_base(vec)->sv_size = 0u;
}

inline _Bool scc_vec_is_allocd(void const *vec) {
    return ((unsigned char const*)vec)[-1];
}

void *scc_vec_impl_clone(void const *vec, size_t elemsize);

/**
 * Clone the provided ``vec``
 *
 * Create a copy of the provided ``vec``, place it on the heap
 * and return a handle to it.
 *
 * \param vec Handle to the ``vec`` to clone.
 *
 * \return A handle to the new ``vec``, or ``NULL`` on failure.
 */
#define scc_vec_clone(vec)                                              \
    scc_vec_impl_clone(vec, sizeof(*(vec)))

/**
 * Iterate over the given ``vec``
 *
 * The macro produces a scope in which \a iter refers to each
 * element in the ``vec``, in turn.
 *
 * \param iter An iterator expanded from ``scc_vec_iter``
 * \param vec The ``vec`` to iterate over
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/vec/simple_iteration.c
 *      :caption: Iterate over a three-element ``vec``, printing each element
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 */
#define scc_vec_foreach(iter, vec)                                      \
    scc_vec_foreach_by(iter, vec, 1)

/**
 * Iterate over the given ``vec`` in reverse.
 *
 * The behavior is identical to that of ``scc_vec_foreach`` except for the
 * ``vec`` being traversed last to first instead of vice versa.
 *
 * \param iter: An iterator expanded from ``scc_vec_iter``
 * \param vec THe ``vec`` instance to iterate over
 */
#define scc_vec_foreach_reversed(iter, vec)                             \
    scc_vec_foreach_reversed_by(iter, vec, 1)

/**
 * Iterate over the vector with an arbitrary stride
 *
 * \param iter An iterator expanded from ``scc_vec_iter``
 * \param vec The ``vec`` instance to iterate over
 * \param by The stride to iterate with
 *
 * \verbatim embed:rst:leading-asterisk
 *
 *  .. literalinclude:: /../examples/vec/iterate_by.c
 *      :caption: Iterate over a three-element ``vec``, printing each element
 *      :start-after: int main
 *      :end-before: }
 *      :language: c
 *
 * \endverbatim
 */
#define scc_vec_foreach_by(iter, vec, by)                               \
    for(void const *scc_pp_cat_expand(scc_vec_end,__LINE__) =           \
            ((iter) = (vec), &(vec)[scc_vec_size(vec)]);                \
        (void const *)(iter) < scc_pp_cat_expand(scc_vec_end,__LINE__); \
        (iter) += (by))

/** Iterate over the ``vec`` in reverse using an arbitrary stride.
 *
 * The behavior is identical to that of ``scc_vec_foreach_by`` except for the
 * ``vec`` being traversed in reverse.
 *
 * \param iter An iterator expanded from ``scc_vec_iter``
 * \param vec The ``vec`` instance to iterate over
 * \param by The stride to iterate with
 */
#define scc_vec_foreach_reversed_by(iter, vec, by)                      \
    for((iter) = &(vec)[scc_vec_size(vec) - 1u]; (iter) >= (vec); (iter) -= (by))

#endif /* SCC_VEC_H */
