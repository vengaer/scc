#include <scc/scc_panic.h>
#include <scc/scc_svec.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_SVEC_MAX_CAPACITY_INCREASE = 4096 };

size_t scc_svec_impl_npad(void const *svec);
size_t scc_svec_size(void const *svec);
size_t scc_svec_capacity(void const *svec);
bool scc_svec_empty(void const *svec);
void scc_svec_pop(void *svec);
void scc_svec_clear(void *vec);
bool scc_svec_is_allocd(void const *svec);

//? .. c:function:: size_t scc_svec_bytesize(size_t capacity, size_t elemsize, size_t npad)
//?
//?     Compute the size in bytes that an svec with the given capacity, element size
//?     and number of padding bytes would have.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param capacity: The capacity of the hypothetical svec
//?     :param elemsize: The size of the elements in the hypothetical svec
//?     :param npad: The number of padding bytes between
//?                  :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>` and
//?                  :ref:`sv_buffer <type_sv_buffer>` in the hypothetical svec
//?     :returns: The size of the hypothetical svec, in bytes
static inline size_t scc_svec_bytesize(size_t capacity, size_t elemsize, size_t npad) {
    return capacity * elemsize + sizeof(struct scc_svec_base) + npad;
}

//? .. size_t scc_svec_calc_new_capacity(size_t current)
//?
//?     Calculate capacity after next size increase of an svec
//?     with the given capacity.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param current: Current capacity of the svec
//?     :returns: The size the svec would have after the next sizeup
static inline size_t scc_svec_calc_new_capacity(size_t current) {
    return current + (current < SCC_SVEC_MAX_CAPACITY_INCREASE ?
                                current : SCC_SVEC_MAX_CAPACITY_INCREASE);
}

//? .. c:function:: struct scc_svec_base *scc_svec_alloc(\
//?        size_t nbytes, size_t nelems, size_t npad)
//?
//?     Allocate and initialize an svec base struct
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param nbytes: Number of bytes to allocate
//?     :param nelems: Number of elements that are to be inserted
//?                    by calling function
//?     :param npad: Number of padding bytes between the
//?                  :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>`
//?                  and :ref:`sv_buffer <type_sv_buffer>` fields
//?                  in the svec.
//?     :returns: Pointer to a newly allocated svec
static struct scc_svec_base *scc_svec_alloc(size_t nbytes, size_t nelems, size_t npad) {
    struct scc_svec_base *v = malloc(nbytes);
    if(!v) {
        return 0;
    }
    v->sv_size = nelems;
    v->sv_buffer[npad - 2u] = npad - 2 * sizeof(unsigned char);
    v->sv_buffer[npad - 1u] = 1u;
    return v;
}

//? .. c:function:: _Bool scc_svec_grow(void *restrict *svec, size_t capacity, size_t elemsize)
//?
//?     Reallocate the svec with increased capacity
//?
//?     :param svec: Address of the handle to the svec to be reallocated
//?     :param cpacity: Desired capacity of the svec
//?     :param elemsize: Size of the elements in the svec
//?     :returns: A :code:`_Bool` indicating whether the capacity of the svec was
//?               successfully increased
//?     :retval true: The svec was successfully reallocated
//?     :retval false: Memory allocation failure
static bool scc_svec_grow(void *restrict *svec, size_t capacity, size_t elemsize) {
    struct scc_svec_base *v;
    size_t const npad = scc_svec_impl_npad(*svec);
    size_t const nbytes = scc_svec_bytesize(capacity, elemsize, npad);

    if(!scc_svec_is_allocd(*svec)) {
        v = scc_svec_alloc(nbytes, scc_svec_size(*svec), npad);
        if(!v) {
            return false;
        }
        memcpy(v->sv_buffer + npad, *svec, scc_svec_size(*svec) * elemsize);
    }
    else {
        v = realloc(scc_svec_impl_base(*svec), nbytes);
        if(!v) {
            return false;
        }
    }

    v->sv_capacity = capacity;
    *svec = v->sv_buffer + npad;
    return true;
}

void *scc_svec_impl_new(void *initvec, size_t offset, size_t capacity) {
    struct scc_svec_base *svec = initvec;
    svec->sv_capacity = capacity;
    unsigned char *buffer = (unsigned char *)initvec + offset;
    buffer[-2] = offset - sizeof(*svec) - 2 * sizeof(*buffer);
    return buffer;
}

void *scc_svec_impl_from(
    void *restrict vec,
    void const *restrict data,
    size_t size,
    size_t elemsize
) {
    if(size > scc_svec_capacity(vec) && !scc_svec_grow(&vec, size, elemsize)) {
        return 0;
    }
    memcpy(vec, data, size * elemsize);
    scc_svec_impl_base(vec)->sv_size = size;
    return vec;
}

bool scc_svec_impl_resize(void *svecaddr, size_t size, size_t elemsize) {
    size_t const currsize = scc_svec_size(*(void **)svecaddr);

    if(!size || currsize == size) {
        return true;
    }
    if(currsize > size) {
        scc_svec_impl_base(*(void **)svecaddr)->sv_size = size;
        return true;
    }

    if(!scc_svec_impl_reserve(svecaddr, size, elemsize)) {
        return false;
    }

    unsigned char *baseaddr = *(void **)svecaddr;
    size_t zsize = (size - currsize) * elemsize;

    memset(baseaddr + currsize * elemsize, 0, zsize);
    scc_svec_impl_base(*(void **)svecaddr)->sv_size = size;
    return true;
}

void scc_svec_impl_erase(void *svec, size_t index, size_t elemsize) {
    --scc_svec_impl_base(svec)->sv_size;
    if(index == scc_svec_size(svec)) {
        /* Last element */
        return;
    }
    size_t const nbytes = (scc_svec_size(svec) - index) * elemsize;
    unsigned char *dstaddr = (unsigned char *)svec + index * elemsize;
    memmove(dstaddr, dstaddr + elemsize, nbytes);
}

void scc_svec_impl_erase_range(void *svec, size_t first, size_t end, size_t elemsize) {
    if(end <= first) {
        return;
    }

    size_t const nelems = end - first;
    assert(nelems);

    scc_svec_impl_base(svec)->sv_size -= nelems;

    if(first == scc_svec_size(svec)) {
        /* Last nelems elements */
        return;
    }
    size_t const nbytes = (scc_svec_size(svec) - first) * elemsize;
    unsigned char *dstaddr = (unsigned char *)svec + first * elemsize;
    unsigned char const *srcaddr = dstaddr + nelems * elemsize;
    memmove(dstaddr, srcaddr, nbytes);
}

size_t scc_svec_impl_at_check(void *svec, size_t index) {
    if(scc_svec_size(svec) <= index) {
        scc_panic("Attempt to access element %zu in vector of size %zu", index, scc_svec_size(svec));
    }
    return index;
}

bool scc_svec_impl_push_ensure_capacity(void *svec, size_t elemsize) {
    size_t const capacity = scc_svec_capacity(*(void **)svec);
    if(scc_svec_size(*(void **)svec) < capacity) {
        return true;
    }
    return scc_svec_grow(svec, scc_svec_calc_new_capacity(capacity), elemsize);
}

bool scc_svec_impl_reserve(void *svec, size_t capacity, size_t elemsize) {
    if(capacity <= scc_svec_capacity(*(void **)svec)) {
        return true;
    }
    return scc_svec_grow(svec, capacity, elemsize);
}

void scc_svec_free(void *svec) {
    if(scc_svec_is_allocd(svec)) {
        free(scc_svec_impl_base(svec));
    }
}

void scc_svec_pop_safe(void *svec) {
    if(!scc_svec_size(svec)) {
        scc_panic("Attempt to pop element from vector of size 0");
    }
    scc_svec_pop(svec);
}

