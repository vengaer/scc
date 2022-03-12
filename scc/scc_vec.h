#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "scc_mem.h"
#include "scc_pp_token.h"

#include <stddef.h>

#define scc_vec(type) type *

/* struct scc_vec_base
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
 *      scc_vec_impl_layout for exact details
 */
struct scc_vec_base {
    size_t sv_size;
    size_t sv_capacity;
    unsigned char sv_buffer[];
};

/* scc_svec_layout
 *
 * Internal use only
 *
 * Actual layout of an instance of scc_vec when
 * instantiated for the given type. The sv_size through
 * sv_capacity fields are identical to the ones in
 * struct scc_vec_base.
 *
 * The primary purpose of sv_npad is to ensure there
 * are enough bytes available to store the padding info.
 * Whether the data is actually stored where the designated
 * field is located in memory depends on the alignment
 * requirements of the given type.
 *
 * unsigned char sv_npad
 *      Used for tracking padding between sv_dynalloc
 *      and sv_buffer.

 * type sv_buffer[]
 *      FAM used for data storage
 */
#define scc_vec_impl_layout(type)                                   \
    struct {                                                        \
        size_t sv_size;                                             \
        size_t sv_capacity;                                         \
        unsigned char sv_npad;                                      \
        type sv_buffer[];                                           \
    }

/* scc_vec_impl_dataoff
 *
 * Internal use only
 *
 * Base-relative offset of the data array
 *
 * type
 *      The type for which the vector is instantiated
 */
#define scc_vec_impl_dataoff(type)                                  \
    offsetof(scc_vec_impl_layout(type), sv_buffer)


/* scc_vec_impl_nullvec
 *
 * Internal use only
 *
 * Creates an empty vector header with automati storage
 * duration. Effectively equivalent to NULL in terms of usage
 *
 * type
 *      The type for which the vector is instantiated
 */
#define scc_vec_impl_nullvec(type)                                  \
    (union {                                                        \
        struct scc_vec_base sv_vec;                                 \
        unsigned char sv_buffer[scc_vec_impl_dataoff(type)];        \
    }){ 0 }.sv_vec

/* scc_vec_impl_base_qual
 *
 * Internal use only
 *
 * Obtain a qualified pointer to the struct
 * scc_vec_base corresponding to the given vec
 *
 * scc_vec(type) vec
 *      The vec for which the base is to be obtained
 *
 * qual
 *      Optional qualifier to apply to the pointer
 */
#define scc_vec_impl_base_qual(vec, qual)                           \
    scc_container_qual(                                             \
        (unsigned char qual *)(vec) - scc_vec_impl_npad(vec),       \
        struct scc_vec_base,                                        \
        sv_buffer,                                                  \
        qual                                                        \
    )

/* scc_vec_impl_base
 *
 * Internal use only
 *
 * Obtain unqualified pointer to the struct
 * scc_vec_base corresponding to the given vec
 *
 * vec
 *      The vec for which the base is to be obtained
 */
#define scc_vec_impl_base(vec)                                          \
    scc_vec_impl_base_qual(vec,)

/* scc_vec_impl_init
 *
 * Internal use only
 *
 * Initialize the empty vector header at nullvec and return
 * a fat pointer to it
 *
 * void *nullvec
 *      Address of the automatically allocated, uninitialized
 *      empty vec to return
 *
 * size_t offset
 *      Base-relative offset of the data array in the given vector
 */
void *scc_vec_impl_init(void *nullvec, size_t offset);

/* scc_vec_init
 *
 * Instantiate a vector for storing instances
 * of the given type and return a fat pointer to it. The
 * initial vector header has automatic storage duration
 *
 * type
 *      The type for which to instantiate the vector
 */
#define scc_vec_init(type)                                              \
    scc_vec_impl_init(                                                  \
        &scc_vec_impl_nullvec(type),                                    \
        scc_vec_impl_dataoff(type)                                      \
    )


/* scc_vec_impl_from
 *
 * Internal use only
 *
 * Initialize a vector with the given values and return
 * a fat pointer to it.
 *
 * size_t offset
 *      Offset of the data array in a vector storing instances of type
 *
 * void const *data
 *      Pointer to array of data to write to the vector
 *
 * size_t size
 *      Number of entries in the data array
 *
 * size_t elemsize
 *      Size of each element in the vector
 */
void *scc_vec_impl_from(
    size_t offset,
    void const *data,
    size_t size,
    size_t elemsize
);

/* scc_vec_impl_from
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
#define scc_vec_from(type, ...)                                         \
    scc_vec_impl_from(                                                  \
        scc_vec_impl_dataoff(type),                                     \
        (type[]){ __VA_ARGS__ },                                        \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                         \
        sizeof(type)                                                    \
    )

/* scc_vec_impl_resize
 *
 * Internal use only
 *
 * Resize the vector to the given size. Empty slots are set to 0
 *
 * void *vec
 *      Handle to the vec instance
 *
 * size_t size
 *      The would-be size of the vector
 *
 * size_t elemsize
 *      Size of each element stored in the vector
 */
_Bool scc_vec_impl_resize(void *vec, size_t size, size_t elemsize);

/* scc_vec_resize
 *
 * Resize the given vector to a certain size. Empty slots are set
 * to 0, slots whose indices are larger than size are stripped
 *
 * scc_vec(type) vec
 *      The vec to be resized
 *
 * size_t size
 *      New size for the vec
 */
#define scc_vec_resize(vec, size)                                       \
    scc_vec_impl_resize(&(vec), size, sizeof(*(vec)))

/* scc_vec_impl_erase
 *
 * Internal use only
 *
 * Erase the element at the given index. Any values beyond the given
 * index are "shifted" down as by memmove
 *
 * void *vec
 *      The vec in which the element is to be erased
 *
 * size_t index
 *      The index of the element to be removed
 *
 * size_t elemsize
 *      Size of each element in the vector
 */
void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize);

/* scc_vec_erase
 *
 * Erase the element at the given index. Values beyond said
 * index are "shifted" down to fill in the hole
 *
 * scc_vec(type) vec
 *      The vec in which the element is to be erased
 *
 * size_t index
 *      The index of the element to be removed
 */
#define scc_vec_erase(vec, index)                                       \
    scc_vec_impl_erase(vec, index, sizeof(*(vec)))

/* scc_vec_impl_erase_range
 *
 * Internal use only
 *
 * Erase elements in the range [first,end). Potential elements beyond
 * the erased range are shifted down as by memmove
 *
 * void *vec
 *      The vec in which the erasure is to be performed
 *
 * size_t first
 *      First element to be erased. The user must ensure that
 *      (first >= 0 && first < scc_vec_size(vec)) is true
 *
 * size_t end
 *      Element _beyond_ the last one to be removed (c.f. C++'s end
 *      iterator).
 *
 * size_t elemsize
 *      Size of each element in the vec
 */
void scc_vec_impl_erase_range(void *vec, size_t first, size_t end, size_t elemsize);

/* scc_vec_erase_range
 *
 * Erase elements in the range [first,end). Potential elements beyond
 * the erased range are shifted down to replace the removed ones
 *
 * void *vec
 *      The vec in which the erasure is to be performed
 *
 * size_t first
 *      First element to be erased. The user must ensure that
 *      (first >= 0 && first < scc_vec_size(vec)) is true
 *
 * size_t end
 *      Element _beyond_ the last one to be removed (c.f. C++'s end
 *      iterator).
 */
#define scc_vec_erase_range(vec, first, end)                            \
    scc_vec_impl_erase_range(vec, first, end, sizeof(*(vec)))

/* scc_vec_impl_at_check
 *
 * Internal use only
 *
 * Verify that the given index is within the range of the given
 * vector. If this is the case, the given index is returned.
 * Otherwise, the program is terminated.
 *
 * void *vec
 *      The vec instance in question
 *
 * size_t index
 *      The index at which the vec is to be accessed
 */
size_t scc_vec_impl_at_check(void *vec, size_t index);

/* scc_vec_at
 *
 * Provides bounds-checked access to the elements of the
 * vector. Equivalent to applying [] to the vec apart from
 * the program being terminated should an OOB access be attempted
 *
 * scc_vec(type) vec
 *      The vec instance
 *
 * size_t index
 *      The index at which the element is to be accessed
 */
#define scc_vec_at(vec, index)                                          \
    ((vec)[scc_vec_impl_at_check(vec, index)])

/* scc_vec_impl_push_ensure_capacity
 *
 * Internal use only
 *
 * Ensure that the vec has a large enough capacity to be
 * able to store at least one more element. If this is not the
 * case at the time of the call, the vector is reallocated.
 *
 * void *vecaddr
 *      Address of the vector for which the capacity is to be asserted
 *
 * size_t elemsize
 *      Size of the element type for which the vector was instantiated
 */
_Bool scc_vec_impl_push_ensure_capacity(void *vecaddr, size_t elemsize);

/* scc_vec_push
 *
 * Push the given value to the back of the vector,
 * reallocating it if needed. Expands to true if the operation
 * was successful
 *
 *  scc_vec(type) *vecaddr
 *      Pointer to the vec for which the value is to be inserted
 *
 *  type' value
 *      Value to be appended to the vector. Subject to implicit
 *      conversion should type and type' differ
 */
#define scc_vec_push(vecaddr, value)                                    \
    (scc_vec_impl_push_ensure_capacity(vecaddr, sizeof(**(vecaddr))) && \
    ((*(vecaddr))[scc_vec_impl_base(*(vecaddr))->sv_size++] = value,1))

/* scc_vec_impl_reserve
 *
 * Internal use only
 *
 * Reserve at least enough bytes of memory to store the requested
 * number of elements. Return true if the request could be honored
 *
 * void *vecaddr
 *      Address of the vec instance in question
 *
 * size_t capacity
 *      The requested capacity, may be less than scc_vec_capacity(vec),
 *      in which case the function returns immediately
 *
 * size_t elemsize
 *      Size of the elements stored in the vec
 */
_Bool scc_vec_impl_reserve(void *vecaddr, size_t capacity, size_t elemsize);

/* scc_vec_reserve
 *
 * Reserve at least enough memory for the vector to be able to store
 * the requested number of elements
 *
 * scc_vec(type) *vecaddr
 *      Address of the vec in question
 *
 * size_t capacity
 *      The requested capacity
 */
#define scc_vec_reserve(vecaddr, capacity)                                  \
    scc_vec_impl_reserve(vecaddr, capacity, sizeof(**(vecaddr)))

/* scc_vec_impl_npad
 *
 * Internal use only
 *
 * Calculate the number of padding bytes between data buffer
 * and the end of the vector base
 *
 * void const *vec
 *      The vec instance in question
 */
inline size_t scc_vec_impl_npad(void const *vec) {
    return ((unsigned char const *)vec)[-1] + sizeof(unsigned char);
}

/* scc_vec_free
 *
 * Reclaim memory allocated for the given vec
 *
 * void *vec
 *      The vec instance to be freed. The pointer is to
 *      be treated as invalid as soon as the function is called
 */
void scc_vec_free(void *vec);

/* scc_vec_size
 *
 * Obtain the size of the given vector
 *
 * void const *vec
 *      The vec instance for which the size is queried
 */
inline size_t scc_vec_size(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_size;
}

/* scc_vec_capacity
 *
 * Obtain the capacity of the given vector
 *
 * void const *vec
 *      The vec instance for which the capacity is queried
 */
inline size_t scc_vec_capacity(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_capacity;
}

/* scc_vec_empty
 *
 * Check if the vec is empty. Equivalent to
 * scc_vec_size(vec) == 0
 *
 * void const *vec
 *      The vec instance
 */
inline _Bool scc_vec_empty(void const *vec) {
    return !scc_vec_size(vec);
}

/* scc_vec_pop
 *
 * Pop the last value off the vector. No
 * bounds checking is performed
 *
 * void *vec
 *      The vec instance
 */
inline void scc_vec_pop(void *vec) {
    --scc_vec_impl_base(vec)->sv_size;
}

/* scc_vec_pop_safe
 *
 * Bounds-checked pop the last value off
 * the vector. Should the vector already
 * be empty at the time of the call, the
 * program is terminated
 *
 * void *vec
 *      The vec instance
 */
void scc_vec_pop_safe(void *vec);

/* scc_vec_clear
 *
 * Clear the contents of the vector
 *
 * void *vec
 *      The vec instance
 */
inline void scc_vec_clear(void *vec) {
    scc_vec_impl_base(vec)->sv_size = 0u;
}

/* scc_vec_foreach
 *
 * Iterate over the given vector. The macro produces
 * a scope in which iter refers to each element in the
 * vector, in order.
 *
 * type *iter
 *      Iterator variable
 *
 * scc_vec(type) svec
 *      The vec instance to iterate over
 */
#define scc_vec_foreach(iter, vec)                                      \
    scc_vec_foreach_by(iter, vec, 1)

/* scc_vec_foreach_reversed
 *
 * Iterate over the vector in reverse. Equivalent to
 * scc_vec_foreach in every way save for the iteration
 * order
 *
 * type *iter
 *      Iterator variable
 *
 * scc_vec(type) vec
 *      The vec instance to iterate over
 */
#define scc_vec_foreach_reversed(iter, vec)                             \
    scc_vec_foreach_reversed_by(iter, vec, 1)

/* scc_vec_foreach_by
 *
 * Iterate over the vector with a given step.
 *
 * type *iter
 *      Iterator variable
 *
 * scc_vec(type) svec
 *      The vec instance to iterate over
 *
 * size_t by
 *      The step size to use in the iteration
 */
#define scc_vec_foreach_by(iter, vec, by)                               \
    for(void const *scc_pp_cat_expand(scc_vec_end,__LINE__) =           \
            ((iter) = (vec), &(vec)[scc_vec_size(vec)]);                \
        (void const *)(iter) < scc_pp_cat_expand(scc_vec_end,__LINE__); \
        (iter) += (by))

/* scc_vec_foreach_reversed_by
 *
 * Iterate over the vector in reverse with a given step.
 *
 * type *iter
 *      Iterator variable
 *
 * scc_vec(type) vec
 *      The vec instance to iterate over
 *
 * size_t by
 *      The step size to use in the iteration
 */
#define scc_vec_foreach_reversed_by(iter, vec, by)                      \
    for((iter) = &(vec)[scc_vec_size(vec) - 1u]; (iter) >= (vec); (iter) -= (by))

#endif /* SCC_VEC_H */
