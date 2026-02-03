#include <scc/deque.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

size_t scc_deque_impl_npad(void const *deque);
size_t scc_deque_capacity(void const *deque);
size_t scc_deque_size(void const *deque);
bool scc_deque_empty(void const *deque);
size_t scc_deque_impl_push_back_index(void *deque);
size_t scc_deque_impl_push_front_index(void *deque);
size_t scc_deque_impl_pop_back_index(void *deque);
size_t scc_deque_impl_pop_front_index(void *deque);
size_t scc_deque_impl_back_index(void const *deque);
void scc_deque_clear(void *deque);

//? .. c:function:: bool scc_deque_get_dynalloc(void const *deque)
//?
//?     Read the :ref:`rd_dynalloc <unsigned_char_rd_dynalloc>` field
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle to the deque in question
//?     :returns: Value of the :code:`rd_dynalloc` field
static inline bool scc_deque_get_dynalloc(void const *deque) {
    return ((unsigned char const *)deque)[-1];
}

//? .. c:function:: void scc_deque_set_dynalloc(void *deque)
//?
//?     Set the :ref:`rd_dynalloc <unsigned_char_rd_dynalloc>` field to 1
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param deque: Handle to the deque
static inline void scc_deque_set_dynalloc(void *deque) {
    ((unsigned char *)deque)[-1] = 1;
}

//? .. c:function:: size_t scc_deque_bytesize(size_t capacity, size_t elemsize, size_t npad)
//?
//?     Calculate the size a deque with the given capacity, elemenet size
//?     and number of paddign bytes would have
//?
//?     .. note::
//?         Internal use only
//?
//?     :param capacity: The would-be capacity of the deque
//?     :param elemsize: The would-be element size of the deque
//?     :param npad: The would-be number of padding bytes between :ref:`rd_dynalloc <unsigned_char_rd_dynalloc>`
//?                  and :ref:`rd_data <type_rd_data>`
static inline size_t scc_deque_bytesize(size_t capacity, size_t elemsize, size_t npad) {
    return capacity * elemsize + sizeof(struct scc_deque_base) + npad;
}

//? .. c:function:: struct scc_deque_base *scc_deque_alloc(size_t capacity, size_t size, size_t elemsize, size_t npad)
//?
//?     Allocate and partially initialize a :ref:`struct scc_deque_base <scc_deque_base>`
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param capacity: The capacity of the deque to be allocated
//?     :param size: The numbes of elements to be written to the newly allocated deque
//?     :param elemsize: The size of each element in the deque
//?     :param npad: The number of padding bytes between :ref:`rd_dynalloc <unsigned_char_rd_dynalloc>`
//?                  and :ref:`rd_data <type_rd_data>`.
//?     :returns: Address of an newly allocated :ref:`struct scc_deque_base <scc_deque_base>`
//?     :retval NULL: On memory allocation failure
//?     :retval Valid address: On successfull allocation
static struct scc_deque_base *scc_deque_alloc(size_t capacity, size_t size, size_t elemsize, size_t npad) {
    size_t const nbytes = scc_deque_bytesize(capacity, elemsize, npad);
    struct scc_deque_base *base = malloc(nbytes);
    if(!base) {
        return 0;
    }
    assert((unsigned char *)base + nbytes == &base->rd_buffer[npad] + capacity * elemsize);
    base->rd_size = size;
    base->rd_capacity = capacity;
    base->rd_buffer[npad - 2u] = npad - 2 * sizeof(unsigned char);
    scc_deque_set_dynalloc(&base->rd_buffer[npad]);
    return base;
}

//? .. c:function:: bool scc_deque_grow(void **dequeaddr, size_t newcap, size_t elemsize)
//?
//?     Reallocate the deque with increased capacity. The elements are repositioned
//?     such that :ref:`rd_begin <size_t_rd_begin>` is 0.
//?
//?     .. note::
//?         Internal use only
//?
//?     :param dequeaddr: Address of the handle to the original deque
//?     :param newcap: The capacity of the deque to be allocated
//?     :param elemsize: Size of each element stored in the deque
//?     :returns: A :code:`_Bool` indicating whether the deque could be reallocated
//?     :retval NULL: Memory allocation failure
//?     :retval Valid address: The deque was successfully reallocated, :c:texpr:`*(void **)dequeaddr`
//?                            is updated accordingly
static bool scc_deque_grow(void **dequeaddr, size_t newcap, size_t elemsize) {
    size_t const npad = scc_deque_impl_npad(*dequeaddr);
    struct scc_deque_base *prev = scc_deque_impl_base(*dequeaddr);
    struct scc_deque_base *base = scc_deque_alloc(newcap, prev->rd_size, elemsize, npad);
    if(!base) {
        return false;
    }

    assert(prev->rd_begin == prev->rd_end);
    unsigned char *data = base->rd_buffer + npad;
    size_t befwrap = prev->rd_capacity - prev->rd_begin;
    size_t first = befwrap * elemsize;
    unsigned char *firstsrc = (unsigned char *)*dequeaddr + prev->rd_begin * elemsize;
    scc_memcpy(data, firstsrc, first);
    if(prev->rd_begin) {
        scc_memcpy(data + first, *dequeaddr, (prev->rd_size - befwrap) * elemsize);
    }
    base->rd_begin = 0;
    base->rd_end = base->rd_size;

    if(scc_deque_get_dynalloc(*dequeaddr)) {
        free(prev);
    }

    *dequeaddr = data;
    return true;
}

void *scc_deque_impl_new(struct scc_deque_base *base, size_t offset, size_t capacity) {
    base->rd_capacity = capacity;
    unsigned char *handle = (unsigned char *)base + offset;
    handle[-2] = offset - sizeof(*base) - 2 * sizeof(*handle);
    return handle;
}

void *scc_deque_impl_new_dyn(size_t dequesz, size_t offset, size_t capacity) {
    struct scc_deque_base *base = calloc(dequesz, sizeof(unsigned char));
    if(!base) {
        return 0;
    }

    void *deque = scc_deque_impl_new(base, offset, capacity);
    scc_deque_set_dynalloc(deque);
    return deque;
}

void scc_deque_free(void *deque) {
    if(scc_deque_get_dynalloc(deque)) {
        free(scc_deque_impl_base(deque));
    }
}

bool scc_deque_impl_prepare_push(void *dequeaddr, size_t elemsize) {
    struct scc_deque_base *base = scc_deque_impl_base(*(void **)dequeaddr);
    if(base->rd_size < base->rd_capacity) {
        return true;
    }
    size_t newcap = base->rd_capacity << 1u;
    return scc_deque_grow(dequeaddr, newcap, elemsize);
}

bool scc_deque_impl_reserve(void *dequeaddr, size_t capacity, size_t elemsize) {
    struct scc_deque_base *base = scc_deque_impl_base(*(void **)dequeaddr);
    if(base->rd_capacity >= capacity) {
        return true;
    }
    if(!scc_bits_is_power_of_2(capacity)) {
        unsigned shifts;
        for(shifts = 0u; capacity; capacity >>= 1u, ++shifts);
        capacity = 1u << shifts;
    }

    assert(scc_bits_is_power_of_2(capacity));
    return scc_deque_grow(dequeaddr, capacity, elemsize);
}

void *scc_deque_impl_clone(void const *deque, size_t elemsize) {
    struct scc_deque_base const *obase = scc_deque_impl_base_qual(deque, const);
    size_t const basesz = (unsigned char const *)deque - (unsigned char const *)obase;
    size_t const bytesz = obase->rd_capacity * elemsize + basesz;
    scc_when_mutating(assert(bytesz > obase->rd_capacity * elemsize));
    struct scc_deque_base *nbase = malloc(bytesz);
    if(!nbase) {
        return 0;
    }

    scc_memcpy(nbase, obase, bytesz);

    unsigned char *ndeque = (unsigned char *)nbase + basesz;
    scc_deque_set_dynalloc(ndeque);
    return ndeque;
}
