#ifndef SCC_DEQUE_H
#define SCC_DEQUE_H

#include "bits.h"
#include "bug.h"
#include "mem.h"

#include <stddef.h>

#ifndef SCC_DEQUE_STATIC_CAPACITY
/**
 * Default capacity of the stack buffer used for the small-size optimized deque.
 *
 * Users may override this value when using the library by providing a preprocessor
 * definition with this name before including the header.
 *
 * \note Must be a power of 2
 */
#define SCC_DEQUE_STATIC_CAPACITY 32
#endif

#if !scc_bits_is_power_of_2(SCC_DEQUE_STATIC_CAPACITY)
#error Stack capacity must be a power of 2
#endif

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque:
 * \endverbatim
 *
 * Expands to an opaque pointer suitable for referring to a
 * ``deque`` storing instances of the the provided \a type.
 *
 * \param type Type of the values to be stored in the deque
 *
 * \verbatim embed:rst:leading-asterisk
 * .. code-block:: c
 *      :caption: Creating a ``deque`` holding ``int`` instances.
 *
 *      scc_deque(int) deque;
 * \endverbatim
 */
#define scc_deque(type) type *

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque_iter:
 * \endverbatim
 *
 * Expands to a pointer suitable for iterating over a deque
 * storing instances of \a type
 *
 * \param type The type stored in the deque
 */
#define scc_deque_iter(type) type *

struct scc_deque_base {
    size_t rd_size;
    size_t rd_capacity;
    size_t rd_begin;
    size_t rd_end;
    unsigned char rd_buffer[];
};

#define scc_deque_impl_layout(type)                                             \
    struct {                                                                    \
        struct {                                                                \
            size_t rd_size;                                                     \
            size_t rd_capacity;                                                 \
            size_t rd_begin;                                                    \
            size_t rd_end;                                                      \
            unsigned char rd_npad;                                              \
            unsigned char rd_dynalloc;                                          \
        } rd0;                                                                  \
        type rd_data[SCC_DEQUE_STATIC_CAPACITY];                                \
    }

#define scc_deque_impl_dataoff(type)                                            \
    sizeof(                                                                     \
        struct {                                                                \
            struct {                                                            \
                size_t rd_size;                                                 \
                size_t rd_capacity;                                             \
                size_t rd_begin;                                                \
                size_t rd_end;                                                  \
                unsigned char rd_npad;                                          \
                unsigned char rd_dynalloc;                                      \
            } rd0;                                                              \
            type rd_data[];                                                     \
        }                                                                       \
    )

#define scc_deque_impl_base_qual(deque, qual)                                   \
    scc_container_qual(                                                         \
        (unsigned char qual *)(deque) - scc_deque_impl_npad(deque),             \
        struct scc_deque_base,                                                  \
        rd_buffer,                                                              \
        qual                                                                    \
    )

#define scc_deque_impl_base(deque)                                              \
    scc_deque_impl_base_qual(deque,)

void *scc_deque_impl_new(struct scc_deque_base *base, size_t offset, size_t capacity);

void *scc_deque_impl_new_dyn(size_t dequesz, size_t offset, size_t capacity);

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque_new:
 * \endverbatim
 *
 * Instantiate a ``deque`` storing instances of the provided \a type. The
 * structure is constructred in the frame of the calling function. Refer to
 * @verbatim embed:rst:inline :ref:`scc_deque_new_dyn <scc_deque_new_dyn>` @endverbatim
 * for the counterpart allocating the structure on the heap.
 *
 * The macro cannot fail.
 *
 * \param type The type to be stored in the deque
 *
 * \return A handle to the constructed deque
 */
#define scc_deque_new(type)                                                     \
    (type *)scc_deque_impl_new(                                                 \
        (void *)&(scc_deque_impl_layout(type)) { 0 },                           \
        scc_deque_impl_dataoff(type),                                           \
        SCC_DEQUE_STATIC_CAPACITY                                               \
    )

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque_new_dyn:
 * \endverbatim
 *
 * Like @verbatim embed:rst:inline :ref:`scc_deque_new <scc_deque_new>` @endverbatim
 * except for the ``_dyn`` version allocating the deque on the heap.
 *
 * \note    Unlike ``scc_deque_new``, ``scc_deque_new_dyn`` may fail. The returned
 *          pointer should be checked against ``NULL``.
 *
 * \param type The type to be stored in the deque
 *
 * \return A handle through which the instantiated deque may be accessed, or ``NULL`` on failure.
 */
#define scc_deque_new_dyn(type)                                                 \
    (type *)scc_deque_impl_new_dyn(                                             \
        sizeof(scc_deque_impl_layout(type)),                                    \
        scc_deque_impl_dataoff(type),                                           \
        SCC_DEQUE_STATIC_CAPACITY                                               \
    )

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque_free:
 * \endverbatim
 *
 * Reclaim memory allocated for the provided ``deque``.
 *
 * The parameter must refer to a valid ``deque``.
 *
 * \param deque Handle referring to the deque to be deallocated
 */
void scc_deque_free(void *deque);

inline size_t scc_deque_impl_npad(void const *deque) {
    return ((unsigned char const *)deque)[-2] + 2 * sizeof(unsigned char);
}

/**
 * Obtain the capacity of the provided ``deque``.
 *
 * \param Handle referring to the deque whose capacity is to be queried.
 *
 * \return Capacity of the provided deque
 */
inline size_t scc_deque_capacity(void const *deque) {
    return scc_deque_impl_base_qual(deque, const)->rd_capacity;
}

/**
 * Obtain the size of the provided ``deque``.
 *
 * \param deque Handle referring to the deque whose size is to be queried.
 *
 * \return Size of the provided deque
 */
inline size_t scc_deque_size(void const *deque) {
    return scc_deque_impl_base_qual(deque, const)->rd_size;
}

/**
 * Check whether the given deque is empty.
 *
 * \param deque The deque in question
 *
 * \return ``true`` if the provided deque is empty, otherwise ``false``
 */
inline _Bool scc_deque_empty(void const *deque) {
    return !scc_deque_size(deque);
}

_Bool scc_deque_impl_prepare_push(void *dequeaddr, size_t elemsize);

inline size_t scc_deque_impl_push_back_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    size_t index = base->rd_end;
    base->rd_end = (base->rd_end + 1u) & (base->rd_capacity - 1u);
    ++base->rd_size;
    return index;
}

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque_push_back:
 * \endverbatim
 *
 * Push a value to the back of the provided ``deque``.
 *
 * The structure is reallocated as required. Thus, pointers into the deque obtained prior to the
 * call must be treated as invalid once the function returns.
 *
 * \param dequeaddr Address of the handle referring to the deque.
 * \param ... The value to push to the deque. Must refer to a single instance of the type stored in
 *            the deque.
 *
 * \return ``true`` on success, ``false`` on failure.
 */
#define scc_deque_push_back(dequeaddr, ...)                                     \
    (scc_deque_impl_prepare_push(dequeaddr, sizeof(**(dequeaddr))) &&           \
    (((*(dequeaddr))[scc_deque_impl_push_back_index(*(dequeaddr))] = __VA_ARGS__),1))

inline size_t scc_deque_impl_push_front_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    base->rd_begin = (base->rd_begin - 1u) & (base->rd_capacity - 1u);
    ++base->rd_size;
    return base->rd_begin;
}

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque_push_front:
 * \endverbatim
 *
 * Push the provided value to the front of the deque.
 *
 * The structure is reallocated as required. Pointers referring to either the deque
 * itself or values stored therein are invalidated by calls to the function.
 *
 * \param dequeaddr Address of the handle referring to the deque.
 * \param ... The value to push to the deque. Must refer to a single instance of the type stored in
 *            the deque.
 *
 * \return ``true`` on success, ``false`` on failure.
 */
#define scc_deque_push_front(dequeaddr, ...)                                    \
    (scc_deque_impl_prepare_push(dequeaddr, sizeof(**(dequeaddr))) &&           \
    ((*(dequeaddr))[scc_deque_impl_push_front_index(*(dequeaddr))] = __VA_ARGS__),1)

inline size_t scc_deque_impl_pop_back_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    base->rd_end = (base->rd_end - 1u) & (base->rd_capacity - 1u);
    --base->rd_size;
    return base->rd_end;
}

/**
 * \verbatim embed:rst:leading-asterisk
 * .. _scc_deque_pop_back:
 * \endverbatim
 *
 * Pop and return the last element in the deque.
 *
 * No bounds checking is performed.
 *
 * \note Due to implementation details, compilers may warn about
 * the result of the expressing being unused unless the poped value
 * is either assigned to an lvalue or cated to ``void``.
 *
 * \param deque Handle referring to the deque
 *
 * \return The element stored at the end of the deque
 */
#define scc_deque_pop_back(deque)                                               \
    (deque)[scc_deque_impl_pop_back_index(deque)]

inline size_t scc_deque_impl_pop_front_index(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    size_t index = base->rd_begin;
    base->rd_begin = (base->rd_begin + 1u) & (base->rd_capacity - 1u);
    --base->rd_size;
    return index;
}

/**
 * Pop and return the first element stored in the deque.
 *
 * Bounds are not checked.
 *
 * \note Due to implementation details, compilers may warn about
 * the result of the expressing being unused unless the poped value
 * is either assigned to an lvalue or cated to ``void``.
 *
 *
 * \see @verbatim embed:rst:inline :ref:`scc_deque_pop_back <scc_deque_pop_back>` @endverbatim
 *
 * \param deque Handle to the deque
 *
 * \return The first element stored in the deque
 */
#define scc_deque_pop_front(deque)                                              \
    (deque)[scc_deque_impl_pop_front_index(deque)]

inline size_t scc_deque_impl_back_index(void const *deque) {
    struct scc_deque_base const *base = scc_deque_impl_base_qual(deque, const);
    return (base->rd_end - 1u) & (base->rd_capacity - 1u);
}

/**
 * Expands to the lvalue stored at the very end of the deque
 *
 * \note No bounds checking is performed, make sure the deque contains
 * at least one value.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/deque/modify_back.c
 *      :caption: Modify the value at the end of the deque
 *      :start-after: scc_deque_new
 *      :end-at: scc_deque_back(deque) == 88
 *      :language: c
 *
 * \endverbatim
 *
 * \param deque Handle referring to the deque
 *
 * \return The last element of the deque
 */
#define scc_deque_back(deque)                                                   \
    (deque)[scc_deque_impl_back_index(deque)]

/**
 * Expands to the lvalue stored at the front of the deque.
 *
 * No bounds checking is performed. Do not call on an empty deque.
 *
 * \param deque The deque
 *
 * \return The value stored at the front of the deque
 */
#define scc_deque_front(deque)                                                  \
    (deque)[scc_deque_impl_base_qual(deque, const)->rd_begin]

/**
 * Clear the provided deque
 *
 * \param deque The deque to clear
 */
inline void scc_deque_clear(void *deque) {
    struct scc_deque_base *base = scc_deque_impl_base(deque);
    base->rd_size = 0u;
    base->rd_begin = 0u;
    base->rd_end = 0u;
}

_Bool scc_deque_impl_reserve(void *dequeaddr, size_t capacity, size_t elemsize);

/**
 * Rserve enough memory that at least ``capacity`` elements can be
 * stored in the provided deque.
 *
 * The actually allocated capacity may exceed the requested one. All pointers to the
 * deque, or elements therein, save for the one whose address is passed to the
 * macro are invalidated.
 *
 * \param dequeaddr Address of the handle referring to the deque
 * \param capacity Desired capacity
 *
 * \return ``true`` on success, otherwise ``false``
 */
#define scc_deque_reserve(dequeaddr, capacity)                                  \
    scc_deque_impl_reserve(dequeaddr, capacity, sizeof(**(dequeaddr)))

void *scc_deque_impl_clone(void const *deque, size_t elemsize);

/**
 * Clone the provided deque.
 *
 * \verbatim embed:rst:leading-asterisk
 *
 * .. literalinclude:: /../examples/deque/clone.c
 *      :caption: Copy a deque
 *      :start-after: main(void)
 *      :end-at: scc_deque_free(copy)
 *      :language: c
 *
 * \endverbatim
 *
 * The returned instance is of the same size and contains copies of all the elements stored
 * in the original deque.
 *
 * \param deque The deque to clone
 *
 * \return A dynamically allocated copy of the provided deque, or ``NULL`` on failure.
 */
#define scc_deque_clone(deque)                                                  \
    scc_deque_impl_clone(deque, sizeof(*(deque)))

inline void *scc_deque_impl_iter_start(void *deque, size_t elemsize) {
    struct scc_deque_base const *base = scc_deque_impl_base_qual(deque, const);
    unsigned char *p = deque;
    return p + elemsize * base->rd_begin;
}

inline void *scc_deque_impl_iter_end(void *deque, size_t elemsize) {
    struct scc_deque_base const *base = scc_deque_impl_base_qual(deque, const);
    unsigned char *p = deque;
    return p + elemsize * base->rd_end;
}

void *scc_deque_impl_iter_next(void *it, void *deque, size_t elemsize);

/**
 * Iterate over elements in the deque
 *
 * Expands to a loop executed once for each element in the deque
 *
 * \param iter Iterator, should be an instance expanded from
 *      @verbatim embed:rst:inline scc_deque_iter @endverbatim
 * \param deque Deque handle
 */
#define scc_deque_foreach(iter, deque)                                          \
    for (void const *deque_iterend =                                            \
        ((void)((iter) = scc_deque_impl_iter_start(deque, sizeof(*(deque)))),   \
            scc_deque_impl_iter_end(deque, sizeof(*(deque))));                  \
         (iter) != deque_iterend;                                               \
         (iter) = scc_deque_impl_iter_next(iter, deque, sizeof(*(deque))))

#endif /* SCC_DEQUE_H */
