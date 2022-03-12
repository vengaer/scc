#ifndef SCC_SVEC_H
#define SCC_SVEC_H

#include "scc_mem.h"
#include "scc_pp_token.h"

#include <stddef.h>

#ifndef SCC_SVEC_STATIC_CAPACITY
enum { SCC_SVEC_STATIC_CAPACITY = 32 };
#endif

#define scc_svec(type) type *

/* struct scc_svec_base
 *
 * Internal use only
 *
 * Base structure of the small vector. Never
 * exposed directly to the user. Instead, all
 * public functions operate on a fat pointer,
 * allowing for direct access using [].
 *
 * size_t sv_size
 *      Size of the vector
 *
 * size_t sv_capacity
 *      Capacity of the vector
 *
 * unsigned char sv_buffer[]
 *      FAM hiding type-specific details. See
 *      scc_svec_impl_layout for exact details
 */
struct scc_svec_base {
    size_t sv_size;
    size_t sv_capacity;
    unsigned char sv_buffer[];
};

/* scc_svec_impl_layout
 *
 * Internal use only
 *
 * Actual layout of an instance of scc_svec when
 * instantiated for the given type. The sv_size through
 * sv_capacity fields are identical to the ones in
 * struct scc_svec_base.
 *
 * The primary purpose of sv_npad and sv_dynalloc
 * is to ensure there are enough bytes available to
 * store the padding and allocation info. Whether the
 * data is actually stored where the designated fields
 * are located in memory depends on the alignment
 * requirements of the given type.
 *
 * unsigned char sv_npad
 *      Used for tracking padding between sv_dynalloc
 *      and sv_buffer.
 *
 * unsigned char sv_dynalloc
 *      Set to 1 if the vector was allocated dynamically
 *
 * type sv_buffer[SCC_SVEC_STATIC_CAPACITY]
 *      Static buffer used while allocated on the stack. Once
 *      allocated dynamically, the buffer is treated as a flexible
 *      array member
 */
#define scc_svec_impl_layout(type)                                      \
    struct {                                                            \
        size_t sv_size;                                                 \
        size_t sv_capacity;                                             \
        unsigned char sv_npad;                                          \
        unsigned char sv_dynalloc;                                      \
        type sv_buffer[SCC_SVEC_STATIC_CAPACITY];                       \
    }

/* scc_svec_impl_initsize
 *
 * Internal use only
 *
 * Size of the stack-allocated svec
 *
 * type
 *      The type for which the vector is to be
 *      instantiated
 */
#define scc_svec_impl_initsize(type)                                    \
    sizeof(scc_svec_impl_layout(type))

/* scc_svec_impl_initvec
 *
 * Internal use only
 *
 * Expands to a buffer with automatic storage
 * duration used for the short vector
 * optimization
 *
 * type
 *      The type for which the vector is to be
 *      instantiated
 */
#define scc_svec_impl_initvec(type)                                     \
    (union {                                                            \
        struct scc_svec_base sv_svec;                                   \
        unsigned char sv_buffer[scc_svec_impl_initsize(type)];          \
    }){ 0 }.sv_svec

/* scc_svec_impl_base_qual
 *
 * Internal use only
 *
 * Obtains a qualified pointer to the struct
 * scc_svec_base corresponding to the given svec
 *
 * scc_svec(type) svec
 *      The svec for which the base is to be obtained
 *
 * qual
 *      Optional qualifier to apply to the pointer
 */
#define scc_svec_impl_base_qual(svec, qual)                             \
    scc_container_qual(                                                 \
        (unsigned char qual *)svec - scc_svec_impl_npad(svec),          \
        struct scc_svec_base,                                           \
        sv_buffer,                                                      \
        qual                                                            \
    )

/* scc_svec_impl_base
 *
 * Internal use only
 *
 * Obtain unqualified pointer to the struct
 * scc_svec_base corresponding to the given svec
 *
 * svec
 *      The svec for which the base is to be obtained
 */
#define scc_svec_impl_base(svec)                                        \
    scc_svec_impl_base_qual(svec,)

/* scc_svec_impl_offset
 *
 * Internal use only
 *
 * Compute the base-relative offset of the data
 * buffer in vectors instantiated for the given
 * type
 *
 * type
 *      The type stored in the vector
 */
#define scc_svec_impl_offset(type)                                      \
    offsetof(scc_svec_impl_layout(type), sv_buffer)

/* scc_svec_impl_init
 *
 * Internal use only
 *
 * Initialize the raw vector at initvec and return
 * a pointer to it
 *
 * void *initvec
 *      Address of the automatically allocated, uninitialized
 *      svec to return
 *
 * size_t offset
 *      Base-relative offset of the data array in the given vector
 *
 * size_t capacity
 *      Capacity of the given vector
 */
void *scc_svec_impl_init(void *initvec, size_t offset, size_t capacity);

/* scc_svec_init
 *
 * Instantiate a small vector for storing instances
 * of the given type and return a pointer to it. The
 * initial vector has automatic storage duration
 *
 * type
 *      The type for which to instantiate the vector
 */
#define scc_svec_init(type)                                             \
    scc_svec_impl_init(                                                 \
        &scc_svec_impl_initvec(type),                                   \
        scc_svec_impl_offset(type),                                     \
        SCC_SVEC_STATIC_CAPACITY                                        \
    )

/* scc_svec_impl_from
 *
 * Internal use only
 *
 * Initialize a vector with the given values and return
 * a fat pointer to it. If the given values do not fit
 * in the automatically allocated parameter, the vector
 * is allocated dynamically immediately
 *
 * void *restrict vec
 *      Pointer to the automatically allocated buffer to use
 *
 * void const *restrict data
 *      Pointer to array of data to write to the vector
 *
 * size_t size
 *      Number of entries in the data array
 *
 * size_t elemsize
 *      Size of each element in the vector
 */
void *scc_svec_impl_from(
        void *restrict vec,
        void const *restrict data,
        size_t size,
        size_t elemsize
    );

/* scc_svec_impl_from
 *
 * Instantiate a vector holding the given values, each
 * interpreted as an instance of the given type.
 *
 * type
 *      The type for which the vector is to be instantiated
 *
 * Ts... args
 *      Values to be written to the vector. Written as if
 *      by memcpy
 */
#define scc_svec_from(type, ...)                                        \
    scc_svec_impl_from(                                                 \
        scc_svec_init(type),                                            \
        (type[]){ __VA_ARGS__ },                                        \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                         \
        sizeof(type)                                                    \
    )

/* scc_svec_impl_resize
 *
 * Internal use only
 *
 * Resize the vector to the given size. Empty slots are set to 0
 *
 * void *svec
 *      Handle to the svec instance
 *
 * size_t size
 *      The would-be size of the vector
 *
 * size_t elemsize
 *      Size of each element stored in the vector
 */
_Bool scc_svec_impl_resize(void *svec, size_t size, size_t elemsize);

/* scc_svec_resize
 *
 * Resize the given vector to a certain size. Empty slots are set
 * to 0, slots whose indices are larger than size are removed
 *
 * scc_svec(type) svec
 *      The svec to be resized
 *
 * size_t size
 *      New size for the svec
 */
#define scc_svec_resize(svec, size)                                     \
    scc_svec_impl_resize(&(svec), size, sizeof(*(svec)))

/* scc_svec_impl_erase
 *
 * Internal use only
 *
 * Erase the element at the given index. Any values beyond the given
 * index are "shifted" down as by memmove
 *
 * void *svec
 *      The svec in which the element is to be erased
 *
 * size_t index
 *      The index of the element to be removed
 *
 * size_t elemsize
 *      Size of each element in the vector
 */
void scc_svec_impl_erase(void *svec, size_t index, size_t elemsize);

/* scc_svec_erase
 *
 * Erase the element at the given index. Values beyond said
 * index are "shifted" down to fill in the hole
 *
 * scc_svec(type) svec
 *      The svec in which the element is to be erased
 *
 * size_t index
 *      The index of the element to be removed
 */
#define scc_svec_erase(svec, index)                                     \
    scc_svec_impl_erase(svec, index, sizeof(*(svec)))

/* scc_svec_impl_erase_range
 *
 * Internal use only
 *
 * Erase elements in the range [first,end). Potential elements beyond
 * the erased range are shifted down as by memmove
 *
 * void *svec
 *      The svec in which the erasure is to be performed
 *
 * size_t first
 *      First element to be erased. The user must ensure that
 *      (first >= 0 && first < scc_svec_size(svec)) is true
 *
 * size_t end
 *      Element _beyond_ the last one to be removed (c.f. C++'s end
 *      iterator).
 *
 * size_t elemsize
 *      Size of each element in the svec
 */
void scc_svec_impl_erase_range(void *svec, size_t first, size_t end, size_t elemsize);

/* scc_svec_erase_range
 *
 * Erase elements in the range [first,end). Potential elements beyond
 * the erased range are shifted down to replace the removed ones
 *
 * void *svec
 *      The svec in which the erasure is to be performed
 *
 * size_t first
 *      First element to be erased. The user must ensure that
 *      (first >= 0 && first < scc_svec_size(svec)) is true
 *
 * size_t end
 *      Element _beyond_ the last one to be removed (c.f. C++'s end
 *      iterator).
 */
#define scc_svec_erase_range(svec, first, end)                          \
    scc_svec_impl_erase_range(svec, first, end, sizeof(*(svec)))

/* scc_svec_impl_at_check
 *
 * Internal use only
 *
 * Verify that the given index is within the range of the given
 * vector. If this is the case, the given index is returned.
 * Otherwise, the program is terminated.
 *
 * void *svec
 *      The svec instance in question
 *
 * size_t index
 *      The index at which the svec is to be accessed
 */
size_t scc_svec_impl_at_check(void *svec, size_t index);

/* scc_svec_at
 *
 * Provides bounds-checked access to the elements of the
 * vector. Equivalent to applying [] to the svec apart from
 * the program being terminated should an OOB access be attempted
 *
 * scc_svec(type) svec
 *      The svec instance
 *
 * size_t index
 *      The index at which the element is to be accessed
 */
#define scc_svec_at(svec, index)                                        \
    ((svec)[scc_svec_impl_at_check(svec, index)])

/* scc_svec_impl_push_ensure_capacity
 *
 * Internal use only
 *
 * Ensure that the svec has a large enough capacity to be
 * able to store at least one more element. If this is not the
 * case at the time of the call, the vector is reallocated.
 *
 * void *svecaddr
 *      The vector for which the capacity is to be asserted
 *
 * size_t elemsize
 *      Size of the element type for which the vector was instantiated
 */
_Bool scc_svec_impl_push_ensure_capacity(void *svecaddr, size_t elemsize);

/* scc_svec_push
 *
 * Push the given value to the back of the vector,
 * reallocating it if needed
 *
 *  scc_svec(type) *svecaddr
 *      Pointer to the svec for which the value is to be inserted
 *
 *  type' value
 *      Value to be appended to the vector. Subject to implicit
 *      conversion should type and type' differ
 */
#define scc_svec_push(svecaddr, value)                                          \
    (scc_svec_impl_push_ensure_capacity((svecaddr), sizeof(**(svecaddr))) &&    \
    ((*(svecaddr))[scc_svec_impl_base(*(svecaddr))->sv_size++] = (value),1))


/* scc_svec_impl_reserve
 *
 * Internal use only
 *
 * Reserve at least enough bytes of memory to store the requested
 * number of elements. Return true if the request could be honored
 *
 * void *svecaddr
 *      Address of the svec instance in question
 *
 * size_t capacity
 *      The requested capacity, may be less than scc_svec_capacity(svec),
 *      in which case the function returns immediately
 *
 * size_t elemsize
 *      Size of the elements stored in the svec
 */
_Bool scc_svec_impl_reserve(void *svecaddr, size_t capacity, size_t elemsize);

/* scc_svec_reserve
 *
 * Reserve at least enough memory for the vector to be able to store
 * the requested number of elements
 *
 * scc_svec(type) *svecaddr
 *      Address of the svec in question
 *
 * size_t capacity
 *      The requested capacity
 */
#define scc_svec_reserve(svecaddr, capacity)                                \
    scc_svec_impl_reserve((svecaddr), capacity, sizeof(**(svecaddr)))

/* scc_svec_impl_npad
 *
 * Internal use only
 *
 * Calculate the number of padding bytes between data buffer
 * and the end of the vector base
 *
 * void const *svec
 *      The svec instance in question
 */
inline size_t scc_svec_impl_npad(void const *svec) {
    return ((unsigned char const *)svec)[-2] + 2 * sizeof(unsigned char);
}

/* scc_svec_free
 *
 * Reclaim memory allocated for the given svec
 *
 * void *svec
 *      The svec instance to be freed. The pointer is to
 *      be treated as invalid as soon as the function is called
 */
void scc_svec_free(void *svec);

/* scc_svec_size
 *
 * Obtain the size of the given vector
 *
 * void const *svec
 *      The svec instance for which the size is queried
 */
inline size_t scc_svec_size(void const *svec) {
    return scc_svec_impl_base_qual(svec, const)->sv_size;
}

/* scc_svec_capacity
 *
 * Obtain the capacity of the given vector
 *
 * void const *svec
 *      The svec instance for which the capacity is queried
 */
inline size_t scc_svec_capacity(void const *svec) {
    return scc_svec_impl_base_qual(svec, const)->sv_capacity;
}

/* scc_svec_empty
 *
 * Check if the svec is empty. Equivalent to
 * scc_svec_size(svec) == 0
 *
 * void const *svec
 *      The svec instance
 */
inline _Bool scc_svec_empty(void const *svec) {
    return !scc_svec_size(svec);
}

/* scc_svec_pop
 *
 * Pop the last value off the vector. No
 * bounds checking is performed
 *
 * void *svec
 *      The svec instance
 */
inline void scc_svec_pop(void *svec) {
    --scc_svec_impl_base(svec)->sv_size;
}

/* scc_svec_pop_safe
 *
 * Bounds-checked pop the last value off
 * the vector. Should the vector already
 * be empty at the time of the call, the
 * program is terminated
 *
 * void *svec
 *      The svec instance
 */
void scc_svec_pop_safe(void *svec);

/* scc_svec_clear
 *
 * Clear the contents of the vector
 *
 * void *svec
 *      The svec instance
 */
inline void scc_svec_clear(void *svec) {
    scc_svec_impl_base(svec)->sv_size = 0u;
}

/* scc_svec_foreach
 *
 * Iterate over the given vector. The macro produces
 * a scope in which iter refers to each element in the
 * vector, in order.
 *
 * type *iter
 *      Iterator variable
 *
 * scc_svec(type) svec
 *      The svec instance to iterate over
 */
#define scc_svec_foreach(iter, svec)                                    \
    scc_svec_foreach_by(iter, svec, 1)

/* scc_svec_foreach_reversed
 *
 * Iterate over the vector in reverse. Equivalent to
 * scc_svec_foreach in every way save for the iteration
 * order
 *
 * type *iter
 *      Iterator variable
 *
 * scc_svec(type) svec
 *      The svec instance to iterate over
 */
#define scc_svec_foreach_reversed(iter, svec)                           \
    scc_svec_foreach_reversed_by(iter, svec, 1)

/* scc_svec_foreach_by
 *
 * Iterate over the vector with a given step.
 *
 * type *iter
 *      Iterator variable
 *
 * scc_svec(type) svec
 *      The svec instance to iterate over
 *
 * size_t by
 *      The step size to use in the iteration
 */
#define scc_svec_foreach_by(iter, svec, by)                             \
    for(void const *scc_pp_cat_expand(scc_svec_end,__LINE__) =          \
            ((iter) = (svec), &(svec)[scc_svec_size(svec)]);            \
        (void const *)(iter) < scc_pp_cat_expand(scc_svec_end,__LINE__);\
        (iter) += (by))

/* scc_svec_foreach_reversed_by
 *
 * Iterate over the vector in reverse with a given step.
 *
 * type *iter
 *      Iterator variable
 *
 * scc_svec(type) svec
 *      The svec instance to iterate over
 *
 * size_t by
 *      The step size to use in the iteration
 */
#define scc_svec_foreach_reversed_by(iter, svec, by)                    \
    for((iter) = &(svec)[scc_svec_size(svec) - 1u]; (iter) >= (svec); (iter) -= (by))

#endif /* SCC_SVEC_H */
