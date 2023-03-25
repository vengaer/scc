#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "bug.h"
#include "mem.h"
#include "pp_token.h"

#include <stddef.h>

//! .. c:enumerator:: SCC_VEC_STATIC_CAPACITY
//!
//!     Capacity of the stack buffer used for small-size
//!     optimization. May be overridden by defining it
//!     before including the vec.h header.
//!
//!     .. note::
//!
//!         Must be greater than 0
#ifndef SCC_VEC_STATIC_CAPACITY
enum { SCC_VEC_STATIC_CAPACITY = 32 };
#endif

scc_static_assert(SCC_VEC_STATIC_CAPACITY > 0);

//! .. c:macro:: scc_vec(type)
//!
//!     Expands to an opaque pointer suitable for storing
//!     a handle to an vec containing instances of the
//!     given :c:texpr:`type`.
//!
//!     :param type: The type to store in the vec
//!
//!     .. code-block:: C
//!         :caption: Create a handle to an vec storing ints
//!
//!         scc_vec(int) vec;
#define scc_vec(type) type *

//? .. _scc_vec_base:
//? .. c:struct:: scc_vec_base
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
struct scc_vec_base {
    size_t sv_size;
    size_t sv_capacity;
    unsigned char sv_buffer[];
};

//? .. _scc_vec_impl_layout:
//? .. c:macro:: scc_vec_impl_layout(type)
//?
//?     Actual layout of an instance of scc_vec when instantiated for
//?     the given :c:texpr:`type`. The :c:texpr:`sv_size` through
//?     :c:texpr:`sv_capacity` fields are identical to those of
//?     ref:`scc_vec_base`.
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
//?         .. c:var:: type sv_buffer[SCC_VEC_STATIC_CAPACITY]
//?
//?             Static buffer used while the vector is located on the stack. Once allocated
//?             dynamically, the buffer is treated as a flexible array member
#define scc_vec_impl_layout(type)                                       \
    struct {                                                            \
        size_t sv_size;                                                 \
        size_t sv_capacity;                                             \
        unsigned char sv_npad;                                          \
        unsigned char sv_dynalloc;                                      \
        type sv_buffer[SCC_VEC_STATIC_CAPACITY];                        \
    }

//? .. _scc_vec_impl_base_qual:
//? .. c:macro:: scc_vec_impl_base_qual(vec, qual)
//?
//?     Obtain qualified pointer to the
//?     :ref:`struct scc_vec_base <scc_vec_base>` corresponding to the
//?     given vec.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: The vec for which the base is to be obtained
//?     :param qual: Qualifiers to apply to the obtained pointer
//?     :returns: Qualified pointer to the
//?               :ref:`struct scc_vec_base <scc_vec_base>` corresponding
//?               to the given vec
#define scc_vec_impl_base_qual(vec, qual)                               \
    scc_container_qual(                                                 \
        (unsigned char qual *)(vec) - scc_vec_impl_npad(vec),           \
        struct scc_vec_base,                                            \
        sv_buffer,                                                      \
        qual                                                            \
    )

//? .. c:macro:: scc_vec_impl_base(vec)
//?
//?     Obtain unqualified pointer to the
//?     :ref:`struct scc_vec_base <scc_vec_base>` corresponding to
//?     the given vec. Equivalent to calling
//?     :ref:`scc_vec_impl_base_qual <scc_vec_impl_base_qual>` without
//?     qualifiers.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: The vec for which the base is to be obtained
//?     :returns: Unqualified pointer to the base of the given vec
#define scc_vec_impl_base(vec)                                          \
    scc_vec_impl_base_qual(vec,)

//? .. c:macro:: scc_vec_impl_offset(type)
//?
//?     Compute the base-relative offset of the
//?     :ref:`sv_buffer <type_sv_buffer>` field in the
//?     :ref:`vec layout <scc_vec_impl_layout>` when
//?     instantiated for the given :c:texpr:`type`.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param type: The type for which the offset is to
//?                  be computed
//?     :returns: Base-relative offset of the buffer field of
//?               an vec instantiated for the given type
#define scc_vec_impl_offset(type)                                        \
    offsetof(scc_vec_impl_layout(type), sv_buffer)

//? .. _scc_vec_impl_new:
//? .. c:function:: void *scc_vec_impl_new(struct scc_vec_base *base, size_t offset, size_t capacity)
//?
//?     Initialize the raw vec at address :c:texpr:`initvec` and return a
//?     pointer to its :ref:`sv_buffer <type_sv_buffer>` member.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param base:  Base addres of the automatically allocated, uninitialized
//?                     vec
//?     :param offset: Base-relative offset of the
//?                    :ref:`sv_buffer <type_sv_buffer>` field in the
//?                    given vector
//?     :param capacity: Capacity of the stack-allocated buffer
//?     :returns: A fat pointer for referring to the newly initialized vec
void *scc_vec_impl_new(struct scc_vec_base *base, size_t offset, size_t capacity);

//? .. c:function:: void *scc_vec_impl_new_dyn(size_t vecsz, size_t offset, size_t capacity)
//?
//?     Like :ref:`scc_vec_impl_new <scc_vec_impl_new>` except for the
//?     vector being allocated on the heap.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vecsz: Size of the vector
//?     :param offset: Base-relative offset of the :ref:`sv_buffer <type_sv_buffer>` field
//?     :param capacity: Capacity of the initial vector
//?     :returns: A fat pointer for referring to the newly initialized vec, or
//?               ``NULL`` on allocation failure
void *scc_vec_impl_new_dyn(size_t vecsz, size_t offset, size_t capacity);

//! .. _scc_vec_new:
//! .. c:function:: void *scc_vec_new(type)
//!
//!     Instantiate a small-size optimized vector for storing instances
//!     of the given type and return a fat pointer referring to it.
//!
//!     The initial vector has automatic storage duration but may be moved
//!     to the heap on subsequent reallocations.
//!
//!     Regardless of size, the returned pointer should be passed to
//!     :ref:`scc_vec_free <scc_vec_free>` for memory reclamation.
//!
//!     .. seealso::
//!
//!         :ref:`scc_vec_new_dyn <scc_vec_new_dyn>` for a dynamically
//!         allocated vector
//!
//!     :param type: The type for which to instantiate the vector
//!     :returns: A handle to the new vec
#define scc_vec_new(type)                                                \
    scc_vec_impl_new(                                                    \
        (void *)&(scc_vec_impl_layout(type)){ 0 },                       \
        scc_vec_impl_offset(type),                                       \
        SCC_VEC_STATIC_CAPACITY                                          \
    )

//! .. _scc_vec_new_dyn:
//! .. c:function:: void *scc_vec_new_dyn(type)
//!
//!     Like :ref:`scc_vec_new <scc_vec_new>` except for the vector
//!     being allocated on the heap
//!
//!     .. note::
//!
//!         Unlike ``scc_vec_new``, ``scc_vec_new_dyn`` may fail. The
//!         returned pointer should always be checked against ``NULL``
//!
//!     :param type: The type for which to instantiate the vector
//!     :returns: A handle to the new vec
#define scc_vec_new_dyn(type)                                            \
    scc_vec_impl_new_dyn(                                                \
        sizeof(scc_vec_impl_layout(type)),                               \
        scc_vec_impl_offset(type),                                       \
        SCC_VEC_STATIC_CAPACITY                                          \
    )

//? .. _scc_vec_impl_from:
//? .. c:function:: void *scc_vec_impl_from(\
//?        void *restrict vec, void const *restrict data, \
//?        size_t size, size_t elemsize)
//?
//?     Initialize an vec with the given values and return
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
void *scc_vec_impl_from(void *restrict vec, void const *restrict data, size_t size, size_t elemsize);

//? .. c:function:: void *scc_vec_impl_from_dyn(size_t vecsz, void const *data, size_t size, size_t elemsize)
//?
//?     Lite :ref:`scc_vec_impl_from <scc_vec_impl_from>` except for the vector being allocated
//?     on the stack.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vecsz: Size of the vector
//?     :param basecap: Initial capacity
//?     :param offset: Base-relative offset of the :ref:`sv_buffer <type_sv_buffer>` field
//?     :param data: Ponter to an array of data to write to the vector
//?     :param size: Number of entries in the data array
//?     :param elemsize: Size of each element in the vector
//?     :returns: Handle to the vector, or ``NULL`` on allocation failure
void *scc_vec_impl_from_dyn(size_t vecsz, size_t basecap, size_t offset, void const *data, size_t size, size_t elemsize);

//! .. _scc_vec_from:
//! .. c:function:: void *scc_vec_from(type, ...)
//!
//!     Instantiate a vector holding the given values, each
//!     interpreted as an instance of the given type.
//!
//!     Variadic parameters are written to the vec in the
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
//!     :returns: An opaque pointer suitable for referring to the vector, or
//!               ``NULL`` on allocation failure
//!
//!     .. code-block:: C
//!         :caption: Initializing an vec with integers 1, 2, 28
//!
//!         scc_vec(int) vec = scc_vec_from(int, 1, 2, 28);
//!         assert(vec[0] == 1);
//!         assert(vec[1] == 2);
//!         assert(vec[2] == 28);
//!         scc_vec_free(vec);
#define scc_vec_from(type, ...)                                             \
    scc_vec_impl_from(                                                      \
        scc_vec_new(type),                                                  \
        (type[]){ __VA_ARGS__ },                                            \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                             \
        sizeof(type)                                                        \
    )

//! .. c:function:: void *scc_vec_from_dyn(type, ...)
//!
//!     Like :ref:`scc_vec_from` except the vector is guaranteed
//!     to be allocated on the heap
//!
//!     :param type: The type for which the vector is to be instantiated
//!     :param ...: Variable number of values used to initialize the vector
//!     :returns: Opaque pointer suitable for referring to the vector, or
//!               ``NULL`` on allocation failure
#define scc_vec_from_dyn(type, ...)                                         \
    scc_vec_impl_from_dyn(                                                  \
        sizeof(scc_vec_impl_layout(type)),                                  \
        SCC_VEC_STATIC_CAPACITY,                                            \
        scc_vec_impl_offset(type),                                          \
        (type[]){ __VA_ARGS__ },                                            \
        scc_arrsize(((type[]){ __VA_ARGS__ })),                             \
        sizeof(type)                                                        \
    )

//? .. c:function:: _Bool scc_vec_impl_resize(\
//?        void *vecaddr, size_t size, size_t elemsize)
//?
//?     Resize the given vector to the given size. Truncating or allocating
//?     new memory as required. Newly allocated slots are set to 0.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vecaddr: Address of the handle to the vec that is to be resized
//?     :param size: The new size of the vector
//?     :param elemsize: Size of the element type of the vector
//?     :returns: A :code:`_Bool` indicating whether the resizing was successful
//?     :retval true: The vec was successfully resized
//?     :retval false: Memory allocation failure
_Bool scc_vec_impl_resize(void *vecaddr, size_t size, size_t elemsize);

//! .. c:function:: _Bool scc_vec_resize(void **vecaddr, size_t size)
//!
//!     Resize the given vec to the specified size. Truncating or
//!     allocating memory as required. Newly allocated elements
//!     are zeroed as if by memset. Potential elements in slots
//!     whose indices are beyond the new size of the vec are stripped.
//!
//!     .. note::
//!
//!         May relocated the vec in memory
//!
//!     :param vecaddr: Address of the handle to the vec to be resized
//!     :param size: Desired size of the vec
//!     :returns: A :code:`_Bool` indicating the outcome
//!     :retval true: Resize successful
//!     :retval false: Memory allocation failure. The vec is left
//!                    unchanged and must still be passed to
//!                    :ref:`scc_vec_free <scc_vec_free>`.
#define scc_vec_resize(vecaddr, size)                                       \
    scc_vec_impl_resize(vecaddr, size, sizeof(**(vecaddr)))

//? .. c:function:: void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize)
//?
//?     Erase the element at the given index. Any values beyond the
//?     erased element are "shifted" down as if by
//?     memmove_
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: The vec in which the element is to be erased
//?     :param index: The index of the element to be erased
//?     :param elemsize: Size of the elements in the vec
//?
//? .. _memmove: https://man7.org/linux/man-pages/man3/memmove.3.html
void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize);

//! .. c:function:: void scc_vec_erase(void *vec, size_t index)
//!
//!     Erase the element at the given index. Values beyond said
//!     index are "shifted" down to fill the hole
//!
//!     :param vec: Handle to the vec in which the element is to be erased
//!     :param index: Index of the element to be removed
#define scc_vec_erase(vec, index)                                       \
    scc_vec_impl_erase(vec, index, sizeof(*(vec)))

//? .. c:function:: void scc_vec_impl_erase_range(\
//?        void *vec, size_t first, size_t end, size_t elemsize)
//?
//?     Erase elements in the range [first,end). Potential elements beyond
//?     the erased range are shifted down as if by memmove_.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: The vec in which the erasure is to be performed
//?     :param first: First welement ot be erased. The user must ensure
//?                   that :c:expr:`first >= 0 && first < scc_vec_size(vec)`
//?                   is :code:`true`
//?     :param end: Element _beyond_ the last one to be removed (c.f.
//?                 C++'s :code:`end` iterator). The user must ensure
//?                 :c:expr:`end <= scc_vec_size(vec)` is :code:`true`
//?     :param elemsize: Size of the elements in the vec
void scc_vec_impl_erase_range(void *vec, size_t first, size_t end, size_t elemsize);

//! .. c:function:: void scc_vec_erase_range(void *vec, size_t first, size_t end)
//!
//!     Erase elements in the range [first,end). Potential elements beyond the
//!     erased range are shifted down to replace the removed ones.
//!
//!     :param vec: Handle to the vec in which the erasure is to be performed
//!     :param first: First element to be erased. The user must ensure
//!                   that :c:expr:`first >= 0 && first < scc_vec_size(vec)`
//!                   is :code:`true`
//!     :param end: Element _beyond_ the last one to be removed (c.f.
//!                 C++'s :code:`end` iterator). The user must ensure
//!                 :c:expr:`end <= scc_vec_size(vec)` is :code:`true`
#define scc_vec_erase_range(vec, first, end)                            \
    scc_vec_impl_erase_range(vec, first, end, sizeof(*(vec)))

//! .. c:function:: type operator[](void *vec, size_t index)
//!
//!     Accesses the indexth element in the given vec without
//!     bounds checking.
//!
//!     :param vec: Handle to vec to index
//!     :param index: Index of the element to return
//!     :returns: The indexth element in the given vec
//!
//!     .. code-block:: C
//!         :caption: Printing the third element in an vec
//!
//!         extern scc_vec(int) vec;
//!         assert(scc_vec_size(vec) >= 3);
//!         printf("The third element is %d\n", vec[2]);
/* Defined implicitly */

//? .. c:function:: _Bool scc_vec_impl_push_ensure_capacity(\
//?        void *vecaddr, size_t elemsize)
//?
//?     Ensure that the vec has a large enough capacity to be able to
//?     store at least one more element. If this is not the at the time
//?     of the call, the vec is reallocated.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vecaddr: Address of the vec handle
//?     :param elemsize: Size of the elements in the given vec
//?     :returns: A :code:`_Bool` indicating whether the vec capacity is
//?               sufficient
//?     :retval true: The vec was already sufficiently large or was
//?                   successfully reallocated to accommodate the
//?                   pending push.
//?     :retval false: The vec capacity was insufficient and memory
//?                    allocation failed
_Bool scc_vec_impl_push_ensure_capacity(void *vecaddr, size_t elemsize);

//! .. c:function:: _Bool scc_vec_push(void *vecaddr, T value)
//!
//!     Push the given value to the back of the vector, reallocating it as
//!     required.
//!
//!     The type :code:`T` must be implicitly convertible to but not necessarily
//!     same the type the vec was instantiated with. The constraint is statically
//!     enforced by the compiler.
//!
//!     :param vecaddr: Address of the handle to the vec
//!     :param value: Value to push to the end of the vector
//!     :returns: A :code:`_Bool` indicating the success of the operation
//!     :retval true: The element was successfully inserted
//!     :retval false: The vec was full at the time of the call and the attempted
//!                    reallocation failed
#define scc_vec_push(vecaddr, value)                                        \
    (scc_vec_impl_push_ensure_capacity(vecaddr, sizeof(**(vecaddr))) &&     \
    ((*(vecaddr))[scc_vec_impl_base(*(vecaddr))->sv_size++] = (value),1))

//? .. c:function:: _Bool scc_vec_impl_reserve(\
//?        void *vecaddr, size_t capacity, size_t elemsize)
//?
//?     Reserve at least enough memory to store the requested number of elements
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vecaddr: Address of the handle of the vec instance in question
//?     :param capacity: The requested capacity, in number of elements. May be
//?                      less than :c:texpr:`scc_vec_capacity(vec)` in which
//?                      case the function returns immediately.
//?     :param elemsize: Size of the elements in the given vec
//?     :returns: A :code:`_Bool` indicating whether enough memory was reserved
//?     :retval true: The capacity was already sufficient, or the required amount
//?                   of memory was successfully allocated
//?     :retval false: The vec capacity at the time of the call was insufficient and
//?                    the necessary memory allocation failed.
_Bool scc_vec_impl_reserve(void *vecaddr, size_t capacity, size_t elemsize);

//! .. c:function:: _Bool scc_vec_reserve(void *vecaddr, size_t capacity)
//!
//!     Reserve at least enough memory that the vector is able to stored
//!     the requested number of elements, reallocating as needed.
//!
//!     :param vecaddr: Address of the handle of the vec in question
//!     :param capacity: The requested capacity
//!     :returns: A :code:`_Bool` indicating whether enough memory was
//!               allocated. On failure, the vec remains unchanged and
//!               must still be passed to :ref:`scc_vec_free <scc_vec_free>`
//!               for memory reclamation.
//!     :retval true: The vec was already sufficiently large or enough memory
//!                   was allocated
//!     :retval false: The vec had to be resized and memory allocation failed
#define scc_vec_reserve(vecaddr, capacity)                                  \
    scc_vec_impl_reserve((vecaddr), capacity, sizeof(**(vecaddr)))

//? .. c:function:: size_t scc_vec_impl_napd(void const *vec)
//?
//?     Calculate the number of padding bytes between data buffer
//?     and the end of the vector base
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: Handle to the vec for which the padding is to be computed
//?     :returns: The number of bytes between the
//?               :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>` and
//?               :ref:`sv_buffer <type_sv_buffer>` fields in the
//?               vec base.
inline size_t scc_vec_impl_npad(void const *vec) {
    return ((unsigned char const *)vec)[-2] + 2 * sizeof(unsigned char);
}

//! .. _scc_vec_free:
//! .. c:function:: void scc_vec_free(void *vec)
//!
//!     Reclaim memory allocated for the given vec. Must only be called with
//!     a handle to a live vec.
//!
//!     :param vec: Handle to the vec to drop
void scc_vec_free(void *vec);

//! .. c:function:: size_t scc_vec_size(void const *vec)
//!
//!     Query the size of the given vec
//!
//!     :param vec: Handle to the vec
//!     :returns: The size of the given vec
inline size_t scc_vec_size(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_size;
}

//! .. c:function:: size_t scc_vec_capacity(void const *vec)
//!
//!     Query the capacity of the given vec
//!
//!     :param vec: Handle to the vec
//!     :returns: The capacity of the given vec
inline size_t scc_vec_capacity(void const *vec) {
    return scc_vec_impl_base_qual(vec, const)->sv_capacity;
}

//! .. c:function:: _Bool scc_vec_empty(void const *vec)
//!
//!     Check whether the given vec is empty
//!
//!     :param vec: Handle to the vec
//!     :returns: :code:`true` is the given vec is empty, otherwise :code:`false`:
inline _Bool scc_vec_empty(void const *vec) {
    return !scc_vec_size(vec);
}

//! .. c:function:: void scc_vec_pop(void *vec)
//!
//!     Pop the last value off the vec. No bounds checking
//!     is performed.
//!
//!     :param vec: Handle to the vec in question
inline void scc_vec_pop(void *vec) {
    --scc_vec_impl_base(vec)->sv_size;
}

//! .. c:function:: void scc_vec_clear(void *vec)
//!
//!     Clear the contents of the vector and set its size
//!     to 0. The capacity is left unchanged.
//!
//!     :param vec: Handle to the vec to clear
inline void scc_vec_clear(void *vec) {
    scc_vec_impl_base(vec)->sv_size = 0u;
}

//? .. c:function:: _Bool scc_vec_is_allocd(void const *vec)
//?
//?     Read the :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>`
//?     field (or equivalent) of the given vec
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: Handle to the vec for which the field is to be read
//?     :returns: Value of the :code:`sv_dyanlloc` field in the base struct
inline _Bool scc_vec_is_allocd(void const *vec) {
    return ((unsigned char const*)vec)[-1];
}

//? .. c:function:: void *scc_vec_impl_clone(void const *vec, size_t elemsize)
//?
//?     Internal clone function
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param vec: ``vec`` handle
//?     :param elemsize: Size of the elements in the vector
//?     :returns: A handle to a new ``vec``, or ``NULL`` on allocation
//?               failure
void *scc_vec_impl_clone(void const *vec, size_t elemsize);

//! .. c:function:: void *scc_vec_clone(void const *vec)
//!
//!     Clone the given ``vec`` and return a handle to the heap-allocated
//!     copy.
//!
//!     :param vec: ``vec`` handle
//!     :returns: A handle to a new ``vec``, or ``NULL`` on allocation
//!               failure
#define scc_vec_clone(vec)                                              \
    scc_vec_impl_clone(vec, sizeof(*(vec)))

//! .. _scc_vec_foreach:
//! .. c:macro:: scc_vec_foreach(iter, vec)
//!
//!     Iterate over the given vec. The macro produces
//!     a scope in which :code:`iter` refers to each element
//!     in the vector, in turn.
//!
//!     :param iter: An instance of a pointer to the type stored in the
//!                  vec. Used for referring to each element, in order.
//!     :param vec: Handle to the vec to iterate over
//!
//!     .. _scc_vec_foreach_example:
//!     .. code-block:: C
//!         :caption: Iterating over a three-element vec, printing each element
//!
//!         scc_vec(int) vec = scc_vec_from(int, 1, 2, 3);
//!         int *iter;
//!
//!         scc_vec_foreach(iter, vec) {
//!             printf("%d ", *iter);  // Prints 1 2 3
//!         }
//!         puts("");
#define scc_vec_foreach(iter, vec)                                      \
    scc_vec_foreach_by(iter, vec, 1)

//! .. c:macro:: scc_vec_foreach_reversed(iter, vec)
//!
//!     Iterate over the given vec in reverse. The behavior is identical to
//!     that of :ref:`scc_vec_foreach <scc_vec_foreach>` except for the vec
//!     being traversed last to first instead of vice versa.
//!
//!     :param iter: An instance of a pointer to the type stored in the
//!                  vec. Used for referring to each element, in reverse order.
//!     :param vec: Handle to the vec to iterate over
//!
//!     .. seealso::
//!
//!         scc_vec_foreach :ref:`example <scc_vec_foreach_example>`

#define scc_vec_foreach_reversed(iter, vec)                             \
    scc_vec_foreach_reversed_by(iter, vec, 1)

//! .. _scc_vec_foreach_by:
//! .. c:macro:: scc_vec_foreach_by(iter, vec, by)
//!
//!     Iterate over the vector with a specified step size
//!
//!     :param iter: Instance of a pointer to the type stored in the
//!                  vec. Used for referring to the elements during traversal
//!     :param vec: Handle to the vec to iterate over
//!     :param by: Size of the step taken each iteration
//!
//!     .. _scc_vec_foreach_by_example:
//!     .. code-block:: C
//!         :caption: Iterating over a six-element vec with step 2
//!
//!         scc_vec(int) vec = scc_vec_from(int, 1, 2, 3, 4, 5, 6);
//!         int *iter;
//!
//!         scc_vec_foreach_by(iter, vec, 2) {
//!             printf("%d ", *iter);  // Prints 1 3 5
//!         }
//!         puts("");
#define scc_vec_foreach_by(iter, vec, by)                               \
    for(void const *scc_pp_cat_expand(scc_vec_end,__LINE__) =           \
            ((iter) = (vec), &(vec)[scc_vec_size(vec)]);                \
        (void const *)(iter) < scc_pp_cat_expand(scc_vec_end,__LINE__); \
        (iter) += (by))

//! .. c:macro:: scc_vec_foreach_reversed_by(iter, vec, by)
//!
//!     Iterate over the vector in reverse with a given step.The behavior is
//!     identical to that of :ref:`scc_vec_foreach_by <scc_vec_foreach_by>`
//!     except for the vec being traversed last to first instead of vice versa.
//!
//!     :param iter: Instance of a pointer to the type stored in the
//!                  vec. Used for referring to the elements during traversal.
//!     :param vec: Handle to the vec in question
//!     :param by: Size of the step taken each iteration
//!
//!     .. seealso::
//!
//!         scc_vec_foreach_by :ref:`example <scc_vec_foreach_by_example>`
#define scc_vec_foreach_reversed_by(iter, vec, by)                      \
    for((iter) = &(vec)[scc_vec_size(vec) - 1u]; (iter) >= (vec); (iter) -= (by))

#endif /* SCC_VEC_H */
