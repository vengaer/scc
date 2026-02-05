#include <scc/vec.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_VEC_MAX_CAPACITY_INCREASE = 4096 };

size_t scc_vec_impl_npad(void const *vec);
size_t scc_vec_size(void const *vec);
size_t scc_vec_capacity(void const *vec);
bool scc_vec_empty(void const *vec);
void scc_vec_clear(void *vec);
bool scc_vec_is_allocd(void const *vec);

//? .. c:function:: size_t scc_vec_bytesize(size_t capacity, size_t elemsize, size_t npad)
//?
//?     Compute the size in bytes that an vec with the given capacity, element size
//?     and number of padding bytes would have.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param capacity: The capacity of the hypothetical vec
//?     :param elemsize: The size of the elements in the hypothetical vec
//?     :param npad: The number of padding bytes between
//?                  :ref:`sv_dynalloc <unsigned_char_sv_dynalloc>` and
//?                  :ref:`sv_buffer <type_sv_buffer>` in the hypothetical vec
//?     :returns: The size of the hypothetical vec, in bytes
static inline size_t scc_vec_bytesize(size_t capacity, size_t elemsize, size_t npad) {
    return capacity * elemsize + sizeof(struct scc_vec_base) + npad;
}

//? .. size_t scc_vec_calc_new_capacity(size_t current)
//?
//?     Calculate capacity after next size increase of an vec
//?     with the given capacity.
//?
//?     .. note::
//?
//?         Internal use only
//?
//?     :param current: Current capacity of the vec
//?     :returns: The size the vec would have after the next sizeup
static inline size_t scc_vec_calc_new_capacity(size_t current) {
    if (current > SCC_VEC_MAX_CAPACITY_INCREASE) {
        return current + SCC_VEC_MAX_CAPACITY_INCREASE;
    }
    return current << 1u | 1u;
}

//? .. c:function:: struct scc_vec_base *scc_vec_alloc(<dnl>
//?        size_t nbytes, size_t nelems, size_t npad)
//?
//?     Allocate and initialize an vec base struct
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
//?                  in the vec.
//?     :returns: Pointer to a newly allocated vec
static struct scc_vec_base *scc_vec_alloc(size_t nbytes, size_t nelems, size_t npad) {
    struct scc_vec_base *v = malloc(nbytes);
    if (!v) {
        return 0;
    }
    v->sv_size = nelems;
    v->sv_buffer[npad - 2u] = npad - 2 * sizeof(unsigned char);
    v->sv_buffer[npad - 1u] = 1u;
    return v;
}

//? .. c:function:: _Bool scc_vec_grow(void *restrict *vec, size_t capacity, size_t elemsize)
//?
//?     Reallocate the vec with increased capacity
//?
//?     :param vec: Address of the handle to the vec to be reallocated
//?     :param cpacity: Desired capacity of the vec
//?     :param elemsize: Size of the elements in the vec
//?     :returns: A :code:`_Bool` indicating whether the capacity of the vec was
//?               successfully increased
//?     :retval true: The vec was successfully reallocated
//?     :retval false: Memory allocation failure
static bool scc_vec_grow(void *restrict *vec, size_t capacity, size_t elemsize) {
    struct scc_vec_base *v;
    size_t const npad = scc_vec_impl_npad(*vec);
    size_t const nbytes = scc_vec_bytesize(capacity, elemsize, npad);

    if (!scc_vec_is_allocd(*vec)) {
        v = scc_vec_alloc(nbytes, scc_vec_size(*vec), npad);
        if (!v) {
            return false;
        }
        memcpy(v->sv_buffer + npad, *vec, scc_vec_size(*vec) * elemsize);
    }
    else {
        v = realloc(scc_vec_impl_base(*vec), nbytes);
        if (!v) {
            return false;
        }
    }

    v->sv_capacity = capacity;
    *vec = v->sv_buffer + npad;
    return true;
}

void *scc_vec_impl_new(struct scc_vec_base *base, size_t offset, size_t capacity) {
    base->sv_capacity = capacity;
    unsigned char *vec = (unsigned char *)base + offset;
    vec[-2] = offset - sizeof(*base) - 2 * sizeof(*vec);
    return vec;
}

void *scc_vec_impl_new_dyn(size_t vecsz, size_t offset, size_t capacity) {
    struct scc_vec_base *base = calloc(vecsz, sizeof(unsigned char));
    if (!base) {
        return 0;
    }

    unsigned char *vec = scc_vec_impl_new(base, offset, capacity);
    vec[-1] = 1;
    return vec;
}

void *scc_vec_impl_from(void *restrict vec, void const *restrict data, size_t size, size_t elemsize) {
    if (size > scc_vec_capacity(vec) && !scc_vec_grow(&vec, size, elemsize)) {
        return 0;
    }
    memcpy(vec, data, size * elemsize);
    scc_vec_impl_base(vec)->sv_size = size;
    return vec;
}

void *scc_vec_impl_from_dyn(size_t basecap, size_t offset, void const *data, size_t size, size_t elemsize) {
    size_t vecsz = offset + basecap * elemsize;
    if (basecap < size) {
        scc_when_mutating(assert(basecap < size));
        vecsz += (size - basecap) * elemsize;
        basecap = size;
        assert(vecsz == offset + size * elemsize);
    }
    unsigned char *vec = scc_vec_impl_new_dyn(vecsz, offset, basecap);
    if (!vec) {
        return 0;
    }

    vec[-1] = 1;
    scc_memcpy(vec, data, size * elemsize);
    scc_vec_impl_base(vec)->sv_size = size;
    return vec;
}

bool scc_vec_impl_resize(void *vecaddr, size_t size, size_t elemsize) {
    size_t const currsize = scc_vec_size(*(void **)vecaddr);

    if (!size) {
        return true;
    }
    if (currsize >= size) {
        scc_vec_impl_base(*(void **)vecaddr)->sv_size = size;
        return true;
    }

    if (!scc_vec_impl_reserve(vecaddr, size, elemsize)) {
        return false;
    }

    unsigned char *baseaddr = *(void **)vecaddr;
    size_t zsize = (size - currsize) * elemsize;
    assert(zsize);

    memset(baseaddr + currsize * elemsize, 0, zsize);
    scc_vec_impl_base(*(void **)vecaddr)->sv_size = size;
    return true;
}

void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize) {
    --scc_vec_impl_base(vec)->sv_size;
    if (index == scc_vec_size(vec)) {
        /* Last element */
        return;
    }
    size_t const nbytes = (scc_vec_size(vec) - index) * elemsize;
    unsigned char *dstaddr = (unsigned char *)vec + index * elemsize;
    memmove(dstaddr, dstaddr + elemsize, nbytes);
}

void scc_vec_impl_erase_range(void *vec, size_t first, size_t end, size_t elemsize) {
    if (end <= first) {
        return;
    }

    size_t const nelems = end - first;
    assert(nelems);

    scc_vec_impl_base(vec)->sv_size -= nelems;

    if (first == scc_vec_size(vec)) {
        /* Last nelems elements */
        return;
    }
    size_t const nbytes = (scc_vec_size(vec) - first) * elemsize;
    unsigned char *dstaddr = (unsigned char *)vec + first * elemsize;
    unsigned char const *srcaddr = dstaddr + nelems * elemsize;
    memmove(dstaddr, srcaddr, nbytes);
}

bool scc_vec_impl_push_ensure_capacity(void *vec, size_t elemsize) {
    size_t const capacity = scc_vec_capacity(*(void **)vec);
    if (scc_vec_size(*(void **)vec) < capacity) {
        return true;
    }
    return scc_vec_grow(vec, scc_vec_calc_new_capacity(capacity), elemsize);
}

bool scc_vec_impl_reserve(void *vec, size_t capacity, size_t elemsize) {
    if (capacity <= scc_vec_capacity(*(void **)vec)) {
        return true;
    }
    return scc_vec_grow(vec, capacity, elemsize);
}

void scc_vec_free(void *vec) {
    if (scc_vec_is_allocd(vec)) {
        free(scc_vec_impl_base(vec));
    }
}

void *scc_vec_impl_clone(void const *vec, size_t elemsize) {
    struct scc_vec_base const *obase = scc_vec_impl_base_qual(vec, const);
    size_t basesz = (unsigned char const *)vec - (unsigned char const *)obase;
    size_t bytesz = basesz + obase->sv_capacity * elemsize;
    struct scc_vec_base *nbase = malloc(bytesz);
    if (!nbase) {
        return 0;
    }

    scc_memcpy(nbase, obase, bytesz);
    return (unsigned char *)nbase + basesz;
}
