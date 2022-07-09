#include <scc/scc_panic.h>
#include <scc/scc_vec.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_VEC_MAX_CAPACITY_INCREASE = 4096 };

size_t scc_vec_impl_npad(void const*vec);
size_t scc_vec_size(void const *vec);
size_t scc_vec_capacity(void const *vec);
bool scc_vec_empty(void const *vec);
void scc_vec_pop(void *vec);
void scc_vec_clear(void *vec);

/* scc_vec_bytesize
 *
 * Return the size a vector with the given capacity, element size
 * and number of padding bytes would have in bytes
 *
 * size_t capacity
 *      The would-be capacity of the vector
 *
 * size_t elemsize
 *      The would-be size of each element in the vector
 *
 * size_t npad
 *      The would-be number of paddign bytes between sv_dynalloc and sv_buffer
 */
static inline size_t scc_vec_bytesize(size_t capacity, size_t elemsize, size_t npad) {
    return capacity * elemsize + sizeof(struct scc_vec_base) + npad;
}

/* scc_vec_calc_new_capacity
 *
 * Calculate capacity to be allocated for upcoming resize
 *
 * size_t current
 *      Current capacity of the vector
 */
static inline size_t scc_vec_calc_new_capacity(size_t current) {
    return current + (current < SCC_VEC_MAX_CAPACITY_INCREASE ?
                                (current | 1) : SCC_VEC_MAX_CAPACITY_INCREASE);
}

/* scc_vec_alloc
 *
 * Allocate and initialize an vec to be filled in later
 *
 * size_t nbytes
 *      Number of bytes to allocate
 *
 * size_t nelemes
 *      Number of elements that are to inserted (done separately)
 *
 * size_t npad
 *      Number of padding bytes between sv_dynalloc and sv_buffer
 */
static struct scc_vec_base *scc_vec_alloc(size_t nbytes, size_t nelems, size_t npad) {
    struct scc_vec_base *v = malloc(nbytes);
    if(!v) {
        return 0;
    }
    v->sv_size = nelems;
    v->sv_buffer[npad - 1u] = npad - sizeof(unsigned char);
    return v;
}

/* scc_vec_grow
 *
 * Reallocate the vector. Return true on success
 *
 * void *restrict *vec
 *      Pointer to the vec instance to be reallocated
 *
 * size_t capacity
 *      The new capacity of the vector
 *
 * size_t elemsize
 *      Size of each element in the vector
 */
static bool scc_vec_grow(void **vec, size_t capacity, size_t elemsize) {
    struct scc_vec_base *v;
    size_t const npad = scc_vec_impl_npad(*vec);
    size_t const nbytes = scc_vec_bytesize(capacity, elemsize, npad);

    if(!scc_vec_capacity(*vec)) {
        v = scc_vec_alloc(nbytes, 0u, npad);
    }
    else {
        v = realloc(scc_vec_impl_base(*vec), nbytes);
    }

    if(!v) {
        return false;
    }

    v->sv_capacity = capacity;
    *vec = v->sv_buffer + npad;
    return true;
}

void *scc_vec_impl_new(void *nullvec, size_t offset) {
    unsigned char *vec = (unsigned char *)nullvec + offset;
    vec[-1] = offset - sizeof(struct scc_vec_base) - sizeof(*vec);
    return vec;
}

void *scc_vec_impl_from(size_t offset, void const *data, size_t size, size_t elemsize) {
    size_t const npad = offset - sizeof(struct scc_vec_base);
    size_t const nbytes = scc_vec_bytesize(size, elemsize, npad);
    struct scc_vec_base *v = scc_vec_alloc(nbytes, size, npad);
    if (!v) {
        return 0;
    }
    v->sv_capacity = size;
    void *buffer = v->sv_buffer + npad;
    memcpy(buffer, data, size * elemsize);
    return buffer;
}

bool scc_vec_impl_resize(void *vec, size_t size, size_t elemsize) {
    size_t const currsize = scc_vec_size(*(void **)vec);

    if(!size || currsize == size) {
        return true;
    }
    if(currsize > size) {
        scc_vec_impl_base(*(void **)vec)->sv_size = size;
        return true;
    }

    if(!scc_vec_impl_reserve(vec, size, elemsize)) {
        return false;
    }

    unsigned char *baseaddr = *(void **)vec;
    size_t zsize = (size - currsize) * elemsize;

    memset(baseaddr + currsize * elemsize, 0, zsize);
    scc_vec_impl_base(*(void **)vec)->sv_size = size;
    return true;
}

void scc_vec_impl_erase(void *vec, size_t index, size_t elemsize) {
    --scc_vec_impl_base(vec)->sv_size;
    if(index == scc_vec_size(vec)) {
        /* Last element */
        return;
    }
    size_t nbytes = (scc_vec_size(vec) - index) * elemsize;
    unsigned char *dstaddr = (unsigned char *)vec + index * elemsize;
    memmove(dstaddr, dstaddr + elemsize, nbytes);
}

void scc_vec_impl_erase_range(void *vec, size_t first, size_t end, size_t elemsize) {
    if(end <= first) {
        return;
    }

    size_t const nelems = end - first;
    scc_vec_impl_base(vec)->sv_size -= nelems;

    if(first == scc_vec_size(vec)) {
        /* Last nelems elements */
        return;
    }
    size_t const nbytes = (scc_vec_size(vec) - first) * elemsize;
    unsigned char *dstaddr = (unsigned char *)vec + first * elemsize;
    unsigned char const *srcaddr = dstaddr + nelems * elemsize;
    memmove(dstaddr, srcaddr, nbytes);
}

size_t scc_vec_impl_at_check(void *vec, size_t index) {
    if(scc_vec_size(vec) <= index) {
        scc_panic("Attempt to access element %zu in vector of size %zu", index, scc_vec_size(vec));
    }
    return index;
}

bool scc_vec_impl_push_ensure_capacity(void *vec, size_t elemsize) {
    size_t capacity = scc_vec_capacity(*(void **)vec);
    if(scc_vec_size(*(void **)vec) < capacity) {
        return true;
    }
    return scc_vec_grow(vec, scc_vec_calc_new_capacity(capacity), elemsize);
}

bool scc_vec_impl_reserve(void *vec, size_t capacity, size_t elemsize) {
    if(capacity <= scc_vec_capacity(*(void **)vec)) {
        return true;
    }
    return scc_vec_grow(vec, capacity, elemsize);
}

void scc_vec_free(void *vec) {
    if(scc_vec_capacity(vec)) {
        free(scc_vec_impl_base(vec));
    }
}

void scc_vec_pop_safe(void *vec) {
    if(!scc_vec_size(vec)) {
        scc_panic("Attempt to pop element from vector of size 0");
    }
    scc_vec_pop(vec);
}

