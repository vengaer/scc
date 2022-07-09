#ifndef SCC_RINGDEQUE_H
#define SCC_RINGDEQUE_H

#include "scc_assert.h"
#include "scc_bits.h"
#include "scc_mem.h"

#include <stddef.h>

#ifndef SCC_RINGDEQUE_STATIC_CAPACITY
//! .. c:enumerator:: SCC_RINGDEQUE_STATIC_CAPACITY
//!
//!     Capacity of the buffer used for small-size optimized
//!     ringdeques. The value may be overridden by defining
//!     it before including the header.
//!
//!     Must be a power of 2
enum { SCC_RINGDEQUE_STATIC_CAPACITY = 32 };
#endif

scc_static_assert(scc_bits_is_power_of_2(SCC_RINGDEQUE_STATIC_CAPACITY));

//! .. c:macro:: scc_ringdeque(type)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to a ringdeque containing the specified type
//!
//!     :param type: The type to store in the ringdeque
#define scc_ringdeque(type) type *

//? .. _scc_ringdeque_base:
//? .. c:struct:: scc_ringdeque_base
//?
//?     Base structure of the ringdeque. Never exposed directly
//?     through the API. All public function operate on a fat
//?     pointer to the :c:texpr:`rd_data` member.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. _size_t_rd_size:
//?     .. c:var:: size_t rd_size
//?
//?         Size of the ringdeque
//?
//?     .. _size_t_rd_capacity:
//?     .. c:var:: size_t rd_capacity
//?
//?         Capacity of the ringdeque
//?
//?     .. _size_t_rd_begin:
//?     .. c:var:: size_t rd_begin
//?
//?         Index of the first slot in the deque
//?
//?     .. _size_t_rd_end:
//?     .. c:var:: size_t rd_end
//?
//?         Index of element beyond the last in the ringdeque.
//?         C.f. C++'s end iterator
//?
//?     .. c:var:: unsigned char rd_buffer[]
//?
//?         FAM hiding details specific to the particular type
//?         stored in the ringdeque. For details, see
//?         :ref:`scc_ringdeque_impl_layout <scc_ringdeque_impl_layout>`
struct scc_ringdeque_base {
    size_t rd_size;
    size_t rd_capacity;
    size_t rd_begin;
    size_t rd_end;
    unsigned char rd_buffer[];
};

//? .. _scc_ringdeque_impl_layout:
//? .. c:macro:: scc_ringdeque_impl_layout(type)
//?
//?     Actual layout of a ringdeque storing instances of the given
//?     :c:texpr:`type`. The :c:texpr:`rd_capacity` through :c:texpr:`rd_end`
//?     fields are identical to those of :ref:`scc_ringdeque_base <scc_ringdeque_base>`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     .. c:struct:: @layout
//?
//?         .. c:var:: size_t rd_size
//?
//?             See :ref:`rd_size <size_t_rd_size>`
//?
//?         .. c:var:: size_t rd_capacity
//?
//?             See :ref:`rd_capacity <size_t_rd_capacity>`
//?
//?         .. c:var:: size_t rd_begin
//?
//?             See :ref:`rd_begin <size_t_rd_begin>`
//?
//?         .. c:var:: size_t rd_end
//?
//?             See :ref:`rd_end <size_t_rd_end>`
//?
//?         .. c:var:: unsigned char rd_npad
//?
//?              Used for tracking potential padding bytes between :c:texpr:`rd_end`
//?              and :c:texpr:`rd_data`
//?
//?         .. _unsigned_char_rd_dynalloc:
//?         .. c:var:: unsigned char rd_dynalloc
//?
//?             Field tracking whether the ringdeque was allocated
//?             dynamically or not
//?
//?         .. _type_rd_data:
//?         .. c:var:: type rd_data[SCC_RINGDEQUE_STATIC_CAPACITY]
//?
//?             Small-size optimized buffer used for storing the
//?             elements in the ringdeque. Should be capacity grow
//?             to the point where the buffer is no longer sufficient,
//?             the ringdeque is moved to the heap.
#define scc_ringdeque_impl_layout(type)                                         \
    struct {                                                                    \
        size_t rd_size;                                                         \
        size_t rd_capacity;                                                     \
        size_t rd_begin;                                                        \
        size_t rd_end;                                                          \
        unsigned char rd_npad;                                                  \
        unsigned char rd_dynalloc;                                              \
        type rd_data[SCC_RINGDEQUE_STATIC_CAPACITY];                            \
    }

//? .. c:macro:: scc_ringdeque_impl_base_qual(deque, qual)
//?
//?     Obtain qualified pointer to the
//?     :ref:`struct scc_ringdeque_base <scc_ringdeque_base>` corresponding
//?     to the given :c:texpr:`deque`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle to the ringdeque whose base address is to be obtained
//?     :param qual: Optional qualifiers to apply to the pointer
//?     :returns: Appropriately qualified address of the
//?               :ref:`struct scc_ringdeque_base <scc_ringdeque_base>`
//?               corresponding to :c:texpr:`deque`.
#define scc_ringdeque_impl_base_qual(deque, qual)                               \
    scc_container_qual(                                                         \
        (unsigned char qual *)(deque) - scc_ringdeque_impl_npad(deque),         \
        struct scc_ringdeque_base,                                              \
        rd_buffer,                                                              \
        qual                                                                    \
    )

//? .. c:macro:: scc_ringdeque_impl_base(deque)
//?
//?     Obtain unqualified pointer to the
//?     :ref:`struct scc_ringdeque_base <scc_ringdeque_base>` corresponding
//?     to the given :c:texpr:`deque`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle to the ringdeque whose base address is to be obtained
//?     :returns: Address of the :ref:`struct scc_ringdeque_base <scc_ringdeque_base>`
//?               corresponding to the given :c:texpr:`deque`.
#define scc_ringdeque_impl_base(deque)                                          \
    scc_ringdeque_impl_base_qual(deque,)

//? .. c:function:: void *scc_ringdeque_impl_new(void *deque, size_t offset, size_t capacity)
//?
//?     Initialize a raw ringdeque at the given address and return a
//?     fat pointer to it.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Address of the ringdeque to be initialized
//?     :param offset: Offset of the :ref:`rd_data <type_rd_data>` member relative :c:texpr:`deque`
//?     :param capacity: The capacity with which the ringdeque at :c:texpr:`deque` was allocated
void *scc_ringdeque_impl_new(void *deque, size_t offset, size_t capacity);

//! .. _scc_ringdeque_new:
//! .. c:function:: void *scc_ringdeque_new(type)
//!
//!     Instantiate a ringdeque storing instances of the given :c:texpr:`type`. The
//!     collection is constructed in the frame of the calling function. For more
//!     information, see :ref:`Scope and Lifetimes <scope_and_lifetimes>`.
//!
//!     The function cannot fail.
//!
//!     :param type: The type to be stored in the ringdeque
//!     :returns: A handle used for referring to the instantiated ringdeque
#define scc_ringdeque_new(type)                                                 \
    scc_ringdeque_impl_new(                                                     \
        &(union {                                                               \
            struct scc_ringdeque_base rd_base;                                  \
            unsigned char rd_buffer[sizeof(scc_ringdeque_impl_layout(type))];   \
        }){ 0 }.rd_base,                                                        \
        offsetof(scc_ringdeque_impl_layout(type), rd_data),                     \
        SCC_RINGDEQUE_STATIC_CAPACITY                                           \
    )

//! .. _scc_ringdeque_free:
//! .. c:function:: void scc_ringdeque_free(void *deque)
//!
//!     Reclaim memory allocated for :c:texpr:`deque`. The parameter must
//!     refer to a valid ringdeque returned by :ref:`scc_ringdeque_new <scc_ringdeque_new>`.
//!
//!     :param deque: Handle to the ringdeque to be freed
void scc_ringdeque_free(void *deque);

//? .. c:function:: size_t scc_ringdeque_impl_npad(void const *deque)
//?
//?     Read the number of padding bytes between the
//?     :ref:`rd_data <type_rd_data>` field and the end of the
//?     :ref:`struct scc_ringdeque_base <scc_ringdeque_base>`.
//?
//?     .. note::
//?         Internal use only
//?
//?     :param deque: Handle to the deque in question
//?     :returns: The number of padding bytes preceding the
//?               :ref:`rd_data <type_rd_data>` field
inline size_t scc_ringdeque_impl_npad(void const *deque) {
    return ((unsigned char const *)deque)[-2] + 2 * sizeof(unsigned char);
}

//! .. c:function:: size_t scc_ringdeque_capacity(void const *deque)
//!
//!     Obtain the capacity of the given ringdeque
//!
//!     :param deque: Handle to the ringdeque for which the capacity is to be queried
//!     :returns: Capacity of the ringdeque corresponding to :c:texpr:`deque`
inline size_t scc_ringdeque_capacity(void const *deque) {
    return scc_ringdeque_impl_base_qual(deque, const)->rd_capacity;
}

//! .. c:function:: size_t scc_ringdeque_size(void const *deque)
//!
//!     Obtain the size of the given ringdeque
//!
//!     :param deque: Handle to the ringdeque for which the size is to be queried
//!     :returns: Size of the ringdeque correspondign to :c:texpr:`deque`
inline size_t scc_ringdeque_size(void const *deque) {
    return scc_ringdeque_impl_base_qual(deque, const)->rd_size;
}

//! .. c:function:: _Bool scc_ringdeque_empty(void const *deque)
//!
//!     Check whether the given ringdeque is empty
//!
//!     :param deque: Handle to the ringdeque in question
//!     :returns: Value indicating whether the ringdeque is empty
//!     :retval true: The ringdeque is empty
//!     :retval false: The ringdeque contains at least one element
inline _Bool scc_ringdeque_empty(void const *deque) {
    return !scc_ringdeque_size(deque);
}

//? .. c:function:: _Bool scc_ringdeque_impl_prepare_push(void *dequeaddr, size_t elemsize)
//?
//?     Ensure that the capacity of the ringdeque at :c:texpr:`*(void **)dequeaddr` is
//?     large enough that at least one more element can be pushed to the back of the
//?     collection.
//?
//?     If the capacity of the ringdeque is not sufficient at the time of the call,
//?     the entire collection is reallocated.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param dequeaddr: Address of the handle used to refer to the ringdeque
//?     :param elemsize: Size of the elements stored in the ringdeque, in bytes
//?     :returns: A :code:`_Bool` indicating whether the ringdeque capacity
//?               is sufficiently large
//?     :retval true: The ringdeque is already sufficiently large to accomodate another
//?                   push. Should this be the case, :c:texpr:`*(void **)dequeaddr`
//?                   is not modified.
//?     :retval true: The ringdeque was successfully reallocated to accomodate the
//?                   pending push. :c:texpr:`*(void **)dequeaddr` is updated accordingly.
//?     :retval false: Allocation failure when resizing
_Bool scc_ringdeque_impl_prepare_push(void *dequeaddr, size_t elemsize);

//? .. c:function:: size_t scc_ringdeque_impl_push_back_index(void *deque)
//?
//?     Obtain the index of the slot just beyond the last in the given ringdeque
//?     and move :ref:`rd_end <size_t_rd_end>` forward.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle to the ringdeque in question
//?     :returns: The index of the slot just beyond the last one in the ringdeque
inline size_t scc_ringdeque_impl_push_back_index(void *deque) {
    struct scc_ringdeque_base *base = scc_ringdeque_impl_base(deque);
    size_t index = base->rd_end;
    base->rd_end = (base->rd_end + 1u) & (base->rd_capacity - 1u);
    ++base->rd_size;
    return index;
}

//! .. c:function:: _Bool scc_ringdeque_push_back(void *dequeaddr, type value)
//!
//!     Push :c:texpr:`value` to the back of the ringdeque, reallocating the
//!     collection if needed.
//!
//!     Any pointers into the ringdeque obtained prior to the call should be treated
//!     as invalid once the function has returned.
//!
//!     :param dequeaddr: Address of the handle to the ringdeque in question. Should
//!                       the ringdeque have to be reallocated, :c:texpr:`*(void **)dequeaddr`
//!                       is updated accordingly.
//!     :param value: Value to push to the back of the ringdeque. Must be
//!                   implicitly convertible to the type the ringdeque was
//!                   instantiated for.
//!     :returns: A :code:`_Bool` indicating whether the push was successful
//!     :retval true: The push succeeded
//!     :retval false: Memory allocation failure
#define scc_ringdeque_push_back(dequeaddr, value)                               \
    (scc_ringdeque_impl_prepare_push(dequeaddr, sizeof(**(dequeaddr))) &&       \
    ((*(dequeaddr))[scc_ringdeque_impl_push_back_index(*(dequeaddr))] = value),1)

//? .. c:function:: size_t scc_ringdeque_impl_push_front_index(void *deque)
//?
//?     Obtain the index of the slot just before the first in the given
//?     ringdeque and move the :ref:`rd_begin <size_t_rd_begin>` backward.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle to the ringdeque in question
//?     :returns: The index of the slot just before the first one in the ringdeque
inline size_t scc_ringdeque_impl_push_front_index(void *deque) {
    struct scc_ringdeque_base *base = scc_ringdeque_impl_base(deque);
    base->rd_begin = (base->rd_begin - 1u) & (base->rd_capacity - 1u);
    ++base->rd_size;
    return base->rd_begin;
}

//! .. c:function:: _Bool scc_ringdeque_push_front(void *dequeaddr, type value)
//!
//!     Push :c:texpr:`value` to the front of the ringdeque, reallocating the
//!     collection if necessary
//!
//!     Any pointers into the ringdeque obtained prior to the call should be treated
//!     as invalid once the function has returned.
//!
//!     :param dequeaddr: Address of the handle to the ringdeque in question. Should
//!                       the ringdeque have to be reallocated, :c:texpr:`*(void **)dequeaddr`
//!                       is updated accordingly.
//!     :param value: Value to push to the front of the ringdeque. Must be
//!                   implicitly convertible to the type the ringdeque was
//!                   instantiated for.
//!     :returns: A :code:`_Bool` indicating whether the push was successful
//!     :retval true: The push succeeded
//!     :retval false: Memory allocation failure
#define scc_ringdeque_push_front(dequeaddr, value)                              \
    (scc_ringdeque_impl_prepare_push(dequeaddr, sizeof(**(dequeaddr))) &&       \
    ((*(dequeaddr))[scc_ringdeque_impl_push_front_index(*(dequeaddr))] = value),1)

//? .. c:function:: size_t scc_ringdeque_impl_pop_back_index(void *deque)
//?
//?     Pop the last element from the ringdeque and return its index in
//?     the ring buffer
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle ot the ringdeque in question
//?     :returns: The index of the just poped element
inline size_t scc_ringdeque_impl_pop_back_index(void *deque) {
    struct scc_ringdeque_base *base = scc_ringdeque_impl_base(deque);
    base->rd_end = (base->rd_end - 1u) & (base->rd_capacity - 1u);
    --base->rd_size;
    return base->rd_end;
}

//! .. c:function:: type scc_ringdeque_pop_back(void *deque)
//!
//!     Pop and return the last element in the ringdeque. No bounds checking
//!     is performed.
//!
//!     Due to implementation details, compilers may warn about
//!     unused result of the expression unless the poped value is
//!     either assigned to an lvalue or casted to :c:expr:`void`.
//!
//!     :param deque: Handle to the ringdeque in question
//!     :returns: The element just poped
#define scc_ringdeque_pop_back(deque)                                           \
    (deque)[scc_ringdeque_impl_pop_back_index(deque)]

//? .. c:function:: size_t scc_ringdeque_impl_pop_front_index(void *deque)
//?
//?     Pop the first element from the ringdeque and return its index in
//?     the ring buffer.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle to the ringdeque
//?     :returns: Index of the poped element
inline size_t scc_ringdeque_impl_pop_front_index(void *deque) {
    struct scc_ringdeque_base *base = scc_ringdeque_impl_base(deque);
    size_t index = base->rd_begin;
    base->rd_begin = (base->rd_begin + 1u) & (base->rd_capacity - 1u);
    --base->rd_size;
    return index;
}

//! .. c:function:: type scc_ringdeque_pop_front(void *deque)
//!
//!     Pop and return the first element in the ringdeque. No bounds
//!     checking is performed.
//!
//!     Due to implementation details, compilers may warn about
//!     unused result of the expression unless the poped value is
//!     either assigned to an lvalue or casted to :c:expr:`void`.
//!
//!     :param deque: Handle to the ringdeque
//!     :returns: The element just poped
#define scc_ringdeque_pop_front(deque)                                          \
    (deque)[scc_ringdeque_impl_pop_front_index(deque)]

//? .. c:function:: size_t scc_ringdeque_impl_back_index(void const *deque)
//?
//?     Compute and return the index of the last element in the given
//?     ringdeque
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Ringdeque handle
//?     :returns: Index of the last element in the ringdeque
inline size_t scc_ringdeque_impl_back_index(void const *deque) {
    struct scc_ringdeque_base const *base = scc_ringdeque_impl_base_qual(deque, const);
    return (base->rd_end - 1u) & (base->rd_capacity - 1u);
}

//! .. c:function:: type scc_ringdeque_back(void *deque)
//!
//!     Expands to an lvalue with the value of the last element
//!     in the ringdeque. No bounds checking is performed, meaning
//!     this must never be called on an empty ringdeque.
//!
//!     :param deque: Handle to the ringdeque
//!     :returns: The last element in the ringdeque
#define scc_ringdeque_back(deque)                                               \
    (deque)[scc_ringdeque_impl_back_index(deque)]

//! .. c:function:: type scc_ringdeque_front(void *deque)
//!
//!     Expands to an lvalue with the value of the first element
//!     in the ringdeque. No bounds cheking is performed, meaning
//!     this must never be called on an empty ringdeque.
//!
//!     :param deque: Ringdeque handle
//!     :returns: The first element in the ringdeque
#define scc_ringdeque_front(deque)                                              \
    (deque)[scc_ringdeque_impl_base_qual(deque, const)->rd_begin]

//! .. c:function:: void scc_ringdeque_clear(void *deque)
//!
//!     Clear the given ringdeque
//!
//!     :param deque: Ringdeque handle
inline void scc_ringdeque_clear(void *deque) {
    struct scc_ringdeque_base *base = scc_ringdeque_impl_base(deque);
    base->rd_size = 0u;
    base->rd_begin = 0u;
    base->rd_end = 0u;
}

//? .. c:function:: _Bool scc_ringdeque_impl_reserve(void *dequeaddr, size_t capacity, size_t elemsize)
//?
//?     Reserve enough memory for storing at least :c:expr:`capacity` elements. See
//?     :ref:`scc_ringdeque_reserve <scc_ringdeque_reserve>` for details.
//?
//?     :param dequeaddr: Address of the ringdeque handle
//?     :param capacity: Requested capacity
//?     :param elemsize: Size of each element stored in the ringdeque
//?     :returns: A :code:`_Bool` indicating whether the request was fulfilled
//?     :retval true: The ringdeque capacity was already sufficiently large
//?     :retval true: The ringdeque was successfully reallocated to satisfy the request
//?     :retval false: Reallocation failed
_Bool scc_ringdeque_impl_reserve(void *dequeaddr, size_t capacity, size_t elemsize);

//! .. _scc_ringdeque_reserve:
//! .. c:function:: _Bool scc_ringdeque_reserve(void *dequeaddr, size_t capacity)
//!
//!     Reserve enough memory that at least :c:expr:`capacity` elements
//!     can be stored in the given ringdeque. The actually allocated
//!     capacity may exceed the requested one.
//!
//!     If the ringdeque has to be reallocated, :c:expr:`*(void **)dequeaddr` is
//!     updated to refer to the new ringdeque. If reallocation fails,
//!     :c:expr:`*(void **)dequeaddr` is left unchanged and the ringdeque must
//!     still be passed to :ref:`scc_ringdeque_free <scc_ringdeque_free>`.
//!
//!     :param dequeaddr: Address of the ringdeque handle
//!     :param capacity: Requested capacity
//!     :returns: A :code:`_Bool` indicating whether the request was fulfilled
//!     :retval true: The ringdeque capacity was already sufficiently large
//!     :retval true: The ringdeque was successfully reallocated to satisfy the request
//!     :retval false: Reallocation failed
#define scc_ringdeque_reserve(dequeaddr, capacity)                              \
    scc_ringdeque_impl_reserve(dequeaddr, capacity, sizeof(**(dequeaddr)))

#endif /* SCC_RINGDEQUE_H */
