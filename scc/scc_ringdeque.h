#ifndef SCC_RINGDEQUE_H
#define SCC_RINGDEQUE_H

#include "scc_mem.h"

#include <stddef.h>

#ifndef SCC_RINGDEQUE_STATIC_CAPACITY
enum { SCC_RINGDEQUE_STATIC_CAPACITY = 32 };
#endif

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
//?     .. c:var:: size_t rd_size
//?
//?         See :ref:`rd_size <size_t_rd_size>`
//?
//?     .. c:var:: size_t rd_capacity
//?
//?         See :ref:`rd_capacity <size_t_rd_capacity>`
//?
//?     .. c:var:: size_t rd_begin
//?
//?         See :ref:`rd_begin <size_t_rd_begin>`
//?
//?     .. c:var:: size_t rd_end
//?
//?         See :ref:`rd_end <size_t_rd_end>`
//?
//?     .. c:var:: unsigned char rd_npad
//?
//?          Used for tracking potential padding bytes between :c:texpr:`rd_end`
//?          and :c:texpr:`rd_data`
//?
//?     .. c:var:: unsigned char rd_dynalloc
//?
//?         Field tracking whether the ringdeque was allocated
//?         dynamically or not
//?
//?     .. _type_rd_data:
//?     .. c:var:: type rd_data[SCC_RINGDEQUE_STATIC_CAPACITY]
//?
//?         Small-size optimized buffer used for storing the
//?         elements in the ringdeque. Should be capacity grow
//?         to the point where the buffer is no longer sufficient,
//?         the ringdeque is moved to the heap.
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
//?         Internal use only
//?
//?     :param deque: Handle to the ringdeque whose base address is to be obtained
//?     :param qual: Optional qualifiers to apply to the pointer
//?     :returns: Appropriated qualified address of the
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
//?         Internal use only
//?
//?     :param deque: Handle to the ringdeque whose base address is to be obtained
//?     :returns: Address of the :ref:`struct scc_ringdeque_base <scc_ringdeque_base>`
//?               corresponding to the given :c:texpr:`deque`.
#define scc_ringdeque_impl_base(deque)                                          \
    scc_ringdeque_impl_base_qual(deque,)

//? .. c:function:: void *scc_ringdeque_impl_init(void *deque, size_t offset, size_t capacity)
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
void *scc_ringdeque_impl_init(void *deque, size_t offset, size_t capacity);

//! .. _scc_ringdeque_init:
//! .. c:function:: void *scc_ringdeque_init(type)
//!
//!     Instantiate a ringdeque storing instances of the given :c:texpr:`type`. The
//!     collection is constructed in the frame of the calling function. For more
//!     information, see :ref:`Scope and Lifetimes <scope_and_lifetimes>`.
//!
//!     The function cannot fail.
//!
//!     :param type: The type to be stored in the ringdeque
//!     :returns: A handle used for referring to the instantiated ringdeque
#define scc_ringdeque_init(type)                                                \
    scc_ringdeque_impl_init(                                                    \
        &(union {                                                               \
            struct scc_ringdeque_base rd_base;                                  \
            unsigned char rd_buffer[sizeof(scc_ringdeque_impl_layout(type))];   \
        }){ 0 }.rd_base,                                                        \
        offsetof(scc_ringdeque_impl_layout(type), rd_data),                     \
        SCC_RINGDEQUE_STATIC_CAPACITY                                           \
    )

//! .. c:function:: void scc_ringdeque_free(void *deque)
//!
//!     Reclaim memory allocated for :c:texpr:`deque`. The parameter must
//!     refer to a valid ringdeque returned by :ref:`scc_ringdeque_init <scc_ringdeque_init>`.
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

#endif /* SCC_RINGDEQUE_H */
