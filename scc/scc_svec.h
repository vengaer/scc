#ifndef SCC_SVEC_H
#define SCC_SVEC_H

#include "bug.h"
#include "mem.h"
#include "pp_token.h"

#include <stddef.h>

//! .. c:enumerator:: SCC_SVEC_STATIC_CAPACITY
//!
//!     Capacity of the stack buffer used for small-size
//!     optimization. May be overridden by defining it
//!     before including the scc_svec.h header.
//!
//!     .. note::
//!
//!         Must be greater than 0
#ifndef SCC_SVEC_STATIC_CAPACITY
enum { SCC_SVEC_STATIC_CAPACITY = 32 };
#endif

scc_static_assert(SCC_SVEC_STATIC_CAPACITY > 0);

//! .. c:macro:: scc_svec(type)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to an svec containing instances of the
//!     given :c:texpr:`type`.
//!
//!     :param type: The type to store in the svec
//!
//!     .. code-block:: C
//!         :caption: Create a handle to an svec storing ints
//!
//!         scc_svec(int) svec;
#define scc_svec(type) type *

//? .. _scc_svec_base:
//? .. c:struct:: scc_svec_base
//?
//?     Base structure of the small vector. Never exposed
//?     directly through the API. Instead, all "public"
//?     functions operate on a fat pointer, allowing
//?     for direct access using []
//?
//?     .. note:: Internal use only
//?
//?     .. _size_t_sv_size:
//?     .. c:var:: size_t sv_size
//?
//?         Size of the vector
//?
//?     .. _size_t_sv_capacity:
//?     .. c:var:: size_t sv_capacity
//?
//?         Capacity of the vector
struct scc_svec_base {
    size_t sv_size;
    size_t sv_capacity;
    unsigned char sv_buffer[];
};

//? .. _scc_svec_impl_layout:
//? .. c:macro:: scc_svec_impl_layout(type)
//?
//?     Actual layout of an instance of scc_svec when instantiated for
//?     the given :c:texpr:`type`. The :c:texpr:`sv_size` through
//?     :c:texpr:`sv_capacity` fields are identical to those of
//?     ref:`scc_svec_base`.
//?
//?     The primary purpose of the :c:texpr:`sv_npad` and
//?     :c:texpr:`sv_dynalloc` fields is to ensure there are enough bytes
//?     available to store the padding and allocation data. Whether the
//?     data is actually stored where the designated fields are located
//?     in memory depends on the alignment requirements of the given type.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type for which the layout is to be obtained
//?
//?     .. c:struct:: @layout
//?
//?         .. c:var:: size_t sv_size
//?
//?             See :ref:`sv_size <size_t_sv_size>`.
//?
//?         .. c:var:: size_t sv_capacity
//?
//?             See :ref:`sv_capacity <size_t_sv_capacity>`.
//?
//?         .. c:var:: unsigned char sv_npad
//?
//?             Used for tracking padding between the
//?             :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>`
//?             and :ref:`sv_buffer <type_sv_buffer>` fields.
//?
//?         .. _unsigned_char_sv_dynalloc:
//?         .. c:var:: unsigned char sv_dynalloc
//?
//?             Set to 1 if the vector was allocated was allocated dynamically
//?
//?         .. _type_sv_buffer:
//?         .. c:var:: type sv_buffer[SCC_SVEC_STATIC_CAPACITY]
//?
//?             Static buffer used while the vector is located on the stack. Once allocated
//?             dynamically, the buffer is treated as a flexible array member
#define scc_svec_impl_layout(type)                                      \
    struct {                                                            \
        size_t sv_size;                                                 \
        size_t sv_capacity;                                             \
        unsigned char sv_npad;                                          \
        unsigned char sv_dynalloc;                                      \
        type sv_buffer[SCC_SVEC_STATIC_CAPACITY];                       \
    }

//? .. c:macro:: scc_svec_impl_initsize(type)
//?
//?     Expands to the total number of bytes an svec instantiated for
//?     the given type occupies on the stack.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type for which the vector is to be instantiated
#define scc_svec_impl_initsize(type)                                    \
    sizeof(scc_svec_impl_layout(type))

//? .. c:macro:: scc_svec_impl_initvec(type)
//?
//?     Expands to a buffer with automatic storage
//?     duration used for the short vector optimization.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type for which the vector is to be instantiated
#define scc_svec_impl_initvec(type)                                     \
    (union {                                                            \
        struct scc_svec_base sv_svec;                                   \
        unsigned char sv_buffer[scc_svec_impl_initsize(type)];          \
    }){ 0 }.sv_svec

//? .. _scc_svec_impl_base_qual:
//? .. c:macro:: scc_svec_impl_base_qual(svec, qual)
//?
//?     Obtain qualified pointer to the
//?     :ref:`struct scc_svec_base <scc_svec_base>` corresponding to the
//?     given svec.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svec: The svec for which the base is to be obtained
//?     :param qual: Qualifiers to apply to the obtained pointer
//?     :returns: Qualified pointer to the
//?               :ref:`struct scc_svec_base <scc_svec_base>` corresponding
//?               to the given svec
#define scc_svec_impl_base_qual(svec, qual)                             \
    scc_container_qual(                                                 \
        (unsigned char qual *)(svec) - scc_svec_impl_npad(svec),        \
        struct scc_svec_base,                                           \
        sv_buffer,                                                      \
        qual                                                            \
    )

//? .. c:macro:: scc_svec_impl_base(svec)
//?
//?     Obtain unqualified pointer to the
//?     :ref:`struct scc_svec_base <scc_svec_base>` corresponding to
//?     the given svec. Equivalent to calling
//?     :ref:`scc_svec_impl_base_qual <scc_svec_impl_base_qual>` without
//?     qualifiers.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svec: The svec for which the base is to be obtained
//?     :returns: Unqualified pointer to the base of the given svec
#define scc_svec_impl_base(svec)                                        \
    scc_svec_impl_base_qual(svec,)

//? .. c:macro:: scc_svec_impl_offset(type)
//?
//?     Compute the base-relative offset of the
//?     :ref:`sv_buffer <type_sv_buffer>` field in the
//?     :ref:`svec layout <scc_svec_impl_layout>` when
//?     instantiated for the given :c:texpr:`type`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type for which the offset is to
//?                  be computed
//?     :returns: Base-relative offset of the buffer field of
//?               an svec instantiated for the given type
#define scc_svec_impl_offset(type)                                      \
    offsetof(scc_svec_impl_layout(type), sv_buffer)

//? .. c:function:: void *scc_svec_impl_new(void *initvec, size_t offset, size_t capacity)
//?
//?     Initialize the raw svec at address :c:texpr:`initvec` and return a
//?     pointer to its :ref:`sv_buffer <type_sv_buffer>` member.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param initvec: Addres of the automatically allocated, uninitialized
//?                     svec
//?     :param offset: Base-relative offset of the
//?                    :ref:`sv_buffer <type_sv_buffer>` field in the
//?                    given vector
//?     :param capacity: Capacity of the stack-allocated buffer
//?     :returns: A fat pointer for referring to the newly initialized svec
void *scc_svec_impl_new(void *initvec, size_t offset, size_t capacity);

//! .. _scc_svec_new:
//! .. c:function:: void *scc_svec_new(type)
//!
//!     Instantiate a small-size optimized vector for storing instances
//!     of the given type and return a fat pointer referring to it.
//!
//!     The initial vector has automatic storage duration but may be moved
//!     to the heap on subsequent reallocations.
//!
//!     Regardless of size, the returned pointer should be passed to
//!     :ref:`scc_svec_free <scc_svec_free>` for memory reclamation.
//!
//!     :param type: The type for which to instantiate the vector
//!     :returns: A handle to the new svec
#define scc_svec_new(type)                                              \
    scc_svec_impl_new(                                                  \
        &scc_svec_impl_initvec(type),                                   \
        scc_svec_impl_offset(type),                                     \
        SCC_SVEC_STATIC_CAPACITY                                        \
    )

//? .. c:function:: void *scc_svec_impl_from(\
//?        void *restrict vec, void const *restrict data, \
//?        size_t size, size_t elemsize)
//?
//?     Initialize an svec with the given values and return
//?     a handle to it. If the given values do not fit in the
//?     automatically allocated :c:texpr:`vec` parameter, the
//?     data is immediately written into dynamically allocated
//?     memory.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: Pointer to the automatically allocated short-optimization
//?                 buffer
//?     :param data: Pointer to an array of data to write to the vector
//?     :param size: Number of entries in the data array
//?     :param elemsize: Size of each element in the vector
//?     :returns: Handle to the vector
void *scc_svec_impl_from(
        void *restrict vec,
        void const *restrict data,
        size_t size,
        size_t elemsize
    );

//! .. c:function:: void *scc_svec_from(type, ...)
//!
//!     Instantiate a vector holding the given values, each
//!     interpreted as an instance of the given type.
//!
//!     Variadic parameters are written to the svec in the
//!     order given.
//!
//!     If the variadic parameters do not all fit in the
//!     small size optimization buffer, they are written
//!     to dynamically allocated memory immediately.
//!
//!     :param type: The type for which the vector is to be instantiated.
//!                  Each variadic parameter is treated as an instance of
//!                  :c:texpr`type` and is subject to implicit conversion
//!                  if required.
//!     :param ...: Variable number of values to initialize the vector with.
//!                 The max number of supported parameters is bounded by the
//!                 compiler. A standard-compliant one should allow at least
//!                 126 separate values.
//!
//!     .. code-block:: C
//!         :caption: Initializing an svec with integers 1, 2, 28
//!
//!         scc_svec(int) svec = scc_svec_from(int, 1, 2, 28);
//!         assert(svec[0] == 1);
//!         assert(svec[1] == 2);
//!         assert(svec[2] == 28);
//!         scc_svec_free(svec);
#define scc_svec_from(type, ...)                                        \
    scc_svec_impl_from(                                                 \
        scc_svec_new(type),                                             \
        (type[]){ __VA_ARGS__ },                                        \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                         \
        sizeof(type)                                                    \
    )

//? .. c:function:: _Bool scc_svec_impl_resize(\
//?        void *svecaddr, size_t size, size_t elemsize)
//?
//?     Resize the given vector to the given size. Truncating or allocating
//?     new memory as required. Newly allocated slots are set to 0.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svecaddr: Address of the handle to the svec that is to be resized
//?     :param size: The new size of the vector
//?     :param elemsize: Size of the element type of the vector
//?     :returns: A :code:`_Bool` indicating whether the resizing was successful
//?     :retval true: The svec was successfully resized
//?     :retval false: Memory allocation failure
_Bool scc_svec_impl_resize(void *svecaddr, size_t size, size_t elemsize);

//! .. c:function:: _Bool scc_svec_resize(void **svecaddr, size_t size)
//!
//!     Resize the given svec to the specified size. Truncating or
//!     allocating memory as required. Newly allocated elements
//!     are zeroed as if by memset. Potential elements in slots
//!     whose indices are beyond the new size of the svec are stripped.
//!
//!     .. note::
//!
//!         May relocated the svec in memory
//!
//!     :param svecaddr: Address of the handle to the svec to be resized
//!     :param size: Desired size of the svec
//!     :returns: A :code:`_Bool` indicating the outcome
//!     :retval true: Resize successful
//!     :retval false: Memory allocation failure. The svec is left
//!                    unchanged and must still be passed to
//!                    :ref:`scc_svec_free <scc_svec_free>`.
#define scc_svec_resize(svecaddr, size)                                     \
    scc_svec_impl_resize(svecaddr, size, sizeof(**(svecaddr)))

//? .. c:function:: void scc_svec_impl_erase(void *svec, size_t index, size_t elemsize)
//?
//?     Erase the element at the given index. Any values beyond the
//?     erased element are "shifted" down as if by
//?     memmove_
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svec: The svec in which the element is to be erased
//?     :param index: The index of the element to be erased
//?     :param elemsize: Size of the elements in the svec
//?
//? .. _memmove: https://man7.org/linux/man-pages/man3/memmove.3.html
void scc_svec_impl_erase(void *svec, size_t index, size_t elemsize);

//! .. c:function:: void scc_svec_erase(void *svec, size_t index)
//!
//!     Erase the element at the given index. Values beyond said
//!     index are "shifted" down to fill the hole
//!
//!     :param svec: Handle to the svec in which the element is to be erased
//!     :param index: Index of the element to be removed
#define scc_svec_erase(svec, index)                                     \
    scc_svec_impl_erase(svec, index, sizeof(*(svec)))

//? .. c:function:: void scc_svec_impl_erase_range(\
//?        void *svec, size_t first, size_t end, size_t elemsize)
//?
//?     Erase elements in the range [first,end). Potential elements beyond
//?     the erased range are shifted down as if by memmove_.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svec: The svec in which the erasure is to be performed
//?     :param first: First welement ot be erased. The user must ensure
//?                   that :c:expr:`first >= 0 && first < scc_svec_size(svec)`
//?                   is :code:`true`
//?     :param end: Element _beyond_ the last one to be removed (c.f.
//?                 C++'s :code:`end` iterator). The user must ensure
//?                 :c:expr:`end <= scc_svec_size(svec)` is :code:`true`
//?     :param elemsize: Size of the elements in the svec
void scc_svec_impl_erase_range(void *svec, size_t first, size_t end, size_t elemsize);

//! .. c:function:: void scc_svec_erase_range(void *svec, size_t first, size_t end)
//!
//!     Erase elements in the range [first,end). Potential elements beyond the
//!     erased range are shifted down to replace the removed ones.
//!
//!     :param svec: Handle to the svec in which the erasure is to be performed
//!     :param first: First element to be erased. The user must ensure
//!                   that :c:expr:`first >= 0 && first < scc_svec_size(svec)`
//!                   is :code:`true`
//!     :param end: Element _beyond_ the last one to be removed (c.f.
//!                 C++'s :code:`end` iterator). The user must ensure
//!                 :c:expr:`end <= scc_svec_size(svec)` is :code:`true`
#define scc_svec_erase_range(svec, first, end)                          \
    scc_svec_impl_erase_range(svec, first, end, sizeof(*(svec)))

//? .. c:function:: size_t scc_svec_impl_at_check(void *svec, size_t index)
//?
//?     Verify that the given index would not index beyond the end of
//?     the svec. If this is the case, the given index is returned,
//?     otherwise, the program is terminated.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svec: Handle to the svec instance in question
//?     :param index: The index at which the svec is to be accessed
size_t scc_svec_impl_at_check(void *svec, size_t index);

//! .. c:function:: type scc_svec_at(void *svec, size_t index)
//!
//!     Accesses the indexth element in the given vector, with
//!     bounds checking. If the index is out-of-bounds, the process
//!     is terminated as if by calling exit_
//!
//!     :param svec: Handle to the svec to index
//!     :param index: Index of the element to be accessed
//!     :returns: The indexth element in the given svec
//!
//! .. _exit: https://man7.org/linux/man-pages/man3/exit.3.html
#define scc_svec_at(svec, index)                                        \
    ((svec)[scc_svec_impl_at_check(svec, index)])

//! .. c:function:: type operator[](void *svec, size_t index)
//!
//!     Accesses the indexth element in the given svec without
//!     bounds checking.
//!
//!     :param svec: Handle to svec to index
//!     :param index: Index of the element to return
//!     :returns: The indexth element in the given svec
//!
//!     .. code-block:: C
//!         :caption: Printing the third element in an svec
//!
//!         extern scc_svec(int) svec;
//!         assert(scc_svec_size(svec) >= 3);
//!         printf("The third element is %d\n", svec[2]);
/* Defined implicitly */

//? .. c:function:: _Bool scc_svec_impl_push_ensure_capacity(\
//?        void *svecaddr, size_t elemsize)
//?
//?     Ensure that the svec has a large enough capacity to be able to
//?     store at least one more element. If this is not the at the time
//?     of the call, the svec is reallocated.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svecaddr: Address of the svec handle
//?     :param elemsize: Size of the elements in the given svec
//?     :returns: A :code:`_Bool` indicating whether the svec capacity is
//?               sufficient
//?     :retval true: The svec was already sufficiently large or was
//?                   successfully reallocated to accommodate the
//?                   pending push.
//?     :retval false: The svec capacity was insufficient and memory
//?                    allocation failed
_Bool scc_svec_impl_push_ensure_capacity(void *svecaddr, size_t elemsize);

//! .. c:function:: _Bool scc_svec_push(void *svecaddr, T value)
//!
//!     Push the given value to the back of the vector, reallocating it as
//!     required.
//!
//!     The type :code:`T` must be implicitly convertible to but not necessarily
//!     same the type the svec was instantiated with. The constraint is statically
//!     enforced by the compiler.
//!
//!     :param svecaddr: Address of the handle to the svec
//!     :param value: Value to push to the end of the vector
//!     :returns: A :code:`_Bool` indicating the success of the operation
//!     :retval true: The element was successfully inserted
//!     :retval false: The svec was full at the time of the call and the attempted
//!                    reallocation failed
#define scc_svec_push(svecaddr, value)                                      \
    (scc_svec_impl_push_ensure_capacity(svecaddr, sizeof(**(svecaddr))) &&  \
    ((*(svecaddr))[scc_svec_impl_base(*(svecaddr))->sv_size++] = (value),1))

//? .. c:function:: _Bool scc_svec_impl_reserve(\
//?        void *svecaddr, size_t capacity, size_t elemsize)
//?
//?     Reserve at least enough memory to store the requested number of elements
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svecaddr: Address of the handle of the svec instance in question
//?     :param capacity: The requested capacity, in number of elements. May be
//?                      less than :c:texpr:`scc_svec_capacity(svec)` in which
//?                      case the function returns immediately.
//?     :param elemsize: Size of the elements in the given svec
//?     :returns: A :code:`_Bool` indicating whether enough memory was reserved
//?     :retval true: The capacity was already sufficient, or the required amount
//?                   of memory was successfully allocated
//?     :retval false: The svec capacity at the time of the call was insufficient and
//?                    the necessary memory allocation failed.
_Bool scc_svec_impl_reserve(void *svecaddr, size_t capacity, size_t elemsize);

//! .. c:function:: _Bool scc_svec_reserve(void *svecaddr, size_t capacity)
//!
//!     Reserve at least enough memory that the vector is able to stored
//!     the requested number of elements, reallocating as needed.
//!
//!     :param svecaddr: Address of the handle of the svec in question
//!     :param capacity: The requested capacity
//!     :returns: A :code:`_Bool` indicating whether enough memory was
//!               allocated. On failure, the svec remains unchanged and
//!               must still be passed to :ref:`scc_svec_free <scc_svec_free>`
//!               for memory reclamation.
//!     :retval true: The svec was already sufficiently large or enough memory
//!                   was allocated
//!     :retval false: The svec had to be resized and memory allocation failed
#define scc_svec_reserve(svecaddr, capacity)                                \
    scc_svec_impl_reserve((svecaddr), capacity, sizeof(**(svecaddr)))

//? .. c:function:: size_t scc_svec_impl_napd(void const *svec)
//?
//?     Calculate the number of padding bytes between data buffer
//?     and the end of the vector base
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svec: Handle to the svec for which the padding is to be computed
//?     :returns: The number of bytes between the
//?               :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>` and
//?               :ref:`sv_buffer <type_sv_buffer>` fields in the
//?               svec base.
inline size_t scc_svec_impl_npad(void const *svec) {
    return ((unsigned char const *)svec)[-2] + 2 * sizeof(unsigned char);
}

//! .. _scc_svec_free:
//! .. c:function:: void scc_svec_free(void *svec)
//!
//!     Reclaim memory allocated for the given svec. Must only be called with
//!     a handle to a live svec.
//!
//!     :param svec: Handle to the svec to drop
void scc_svec_free(void *svec);

//! .. c:function:: size_t scc_svec_size(void const *svec)
//!
//!     Query the size of the given svec
//!
//!     :param svec: Handle to the svec
//!     :returns: The size of the given svec
inline size_t scc_svec_size(void const *svec) {
    return scc_svec_impl_base_qual(svec, const)->sv_size;
}

//! .. c:function:: size_t scc_svec_capacity(void const *svec)
//!
//!     Query the capacity of the given svec
//!
//!     :param svec: Handle to the svec
//!     :returns: The capacity of the given svec
inline size_t scc_svec_capacity(void const *svec) {
    return scc_svec_impl_base_qual(svec, const)->sv_capacity;
}

//! .. c:function:: _Bool scc_svec_empty(void const *svec)
//!
//!     Check whether the given svec is empty
//!
//!     :param svec: Handle to the svec
//!     :returns: :code:`true` is the given svec is empty, otherwise :code:`false`:
inline _Bool scc_svec_empty(void const *svec) {
    return !scc_svec_size(svec);
}

//! .. c:function:: void scc_svec_pop(void *svec)
//!
//!     Pop the last value off the svec. No bounds checking
//!     is performed.
//!
//!     :param svec: Handle to the svec in question
inline void scc_svec_pop(void *svec) {
    --scc_svec_impl_base(svec)->sv_size;
}

//! .. c:function:: void scc_svec_pop_safe(void *svec)
//!
//!     Pop the last value off the vector, with bounds
//!     checking. Should the vector already be empty, the
//!     program is terminated as if by calling exit_.
//!
//!     :param svec: Handle to svec to pop from
void scc_svec_pop_safe(void *svec);

//! .. c:function:: void scc_svec_clear(void *svec)
//!
//!     Clear the contents of the vector and set its size
//!     to 0. The capacity is left unchanged.
//!
//!     :param svec: Handle to the svec to clear
inline void scc_svec_clear(void *svec) {
    scc_svec_impl_base(svec)->sv_size = 0u;
}

//? .. c:function:: _Bool scc_svec_is_allocd(void const *svec)
//?
//?     Read the :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>`
//?     field (or equivalent) of the given svec
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param svec: Handle to the svec for which the field is to be read
//?     :returns: Value of the :code:`sv_dyanlloc` field in the base struct
inline _Bool scc_svec_is_allocd(void const *svec) {
    return ((unsigned char const*)svec)[-1];
}

//! .. _scc_svec_foreach:
//! .. c:macro:: scc_svec_foreach(iter, svec)
//!
//!     Iterate over the given svec. The macro produces
//!     a scope in which :code:`iter` refers to each element
//!     in the vector, in turn.
//!
//!     :param iter: An instance of a pointer to the type stored in the
//!                  svec. Used for referring to each element, in order.
//!     :param svec: Handle to the svec to iterate over
//!
//!     .. _scc_svec_foreach_example:
//!     .. code-block:: C
//!         :caption: Iterating over a three-element svec, printing each element
//!
//!         scc_svec(int) svec = scc_svec_from(int, 1, 2, 3);
//!         int *iter;
//!
//!         scc_svec_foreach(iter, svec) {
//!             printf("%d ", *iter);  // Prints 1 2 3
//!         }
//!         puts("");
#define scc_svec_foreach(iter, svec)                                    \
    scc_svec_foreach_by(iter, svec, 1)

//! .. c:macro:: scc_svec_foreach_reversed(iter, svec)
//!
//!     Iterate over the given svec in reverse. The behavior is identical to
//!     that of :ref:`scc_svec_foreach <scc_svec_foreach>` except for the svec
//!     being traversed last to first instead of vice versa.
//!
//!     :param iter: An instance of a pointer to the type stored in the
//!                  svec. Used for referring to each element, in reverse order.
//!     :param svec: Handle to the svec to iterate over
//!
//!     .. seealso::
//!
//!         scc_svec_foreach :ref:`example <scc_svec_foreach_example>`

#define scc_svec_foreach_reversed(iter, svec)                           \
    scc_svec_foreach_reversed_by(iter, svec, 1)

//! .. _scc_svec_foreach_by:
//! .. c:macro:: scc_svec_foreach_by(iter, svec, by)
//!
//!     Iterate over the vector with a specified step size
//!
//!     :param iter: Instance of a pointer to the type stored in the
//!                  svec. Used for referring to the elements during traversal
//!     :param svec: Handle to the svec to iterate over
//!     :param by: Size of the step taken each iteration
//!
//!     .. _scc_svec_foreach_by_example:
//!     .. code-block:: C
//!         :caption: Iterating over a six-element svec with step 2
//!
//!         scc_svec(int) svec = scc_svec_from(int, 1, 2, 3, 4, 5, 6);
//!         int *iter;
//!
//!         scc_svec_foreach_by(iter, svec, 2) {
//!             printf("%d ", *iter);  // Prints 1 3 5
//!         }
//!         puts("");
#define scc_svec_foreach_by(iter, svec, by)                             \
    for(void const *scc_pp_cat_expand(scc_svec_end,__LINE__) =          \
            ((iter) = (svec), &(svec)[scc_svec_size(svec)]);            \
        (void const *)(iter) < scc_pp_cat_expand(scc_svec_end,__LINE__);\
        (iter) += (by))

//! .. c:macro:: scc_svec_foreach_reversed_by(iter, svec, by)
//!
//!     Iterate over the vector in reverse with a given step.The behavior is
//!     identical to that of :ref:`scc_svec_foreach_by <scc_svec_foreach_by>`
//!     except for the svec being traversed last to first instead of vice versa.
//!
//!     :param iter: Instance of a pointer to the type stored in the
//!                  svec. Used for referring to the elements during traversal.
//!     :param svec: Handle to the svec in question
//!     :param by: Size of the step taken each iteration
//!
//!     .. seealso::
//!
//!         scc_svec_foreach_by :ref:`example <scc_svec_foreach_by_example>`
#define scc_svec_foreach_reversed_by(iter, svec, by)                    \
    for((iter) = &(svec)[scc_svec_size(svec) - 1u]; (iter) >= (svec); (iter) -= (by))

#endif /* SCC_SVEC_H */
