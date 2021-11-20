#include <scc/scc_svec.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_SVEC_MAX_CAPACITY_INCREASE = 4096 };

size_t scc_svec_size(void const *svec);
size_t scc_svec_impl_npad(void const *svec);
size_t scc_svec_capacity(void const *svec);

static bool scc_svec_is_allocd(void const *svec);
static size_t scc_svec_memsize(size_t capacity, size_t elemsize, size_t npad);
static size_t scc_svec_calc_new_capacity(size_t current);
static struct scc_svec *scc_svec_alloc(size_t nbytes, size_t nelems, size_t npad);
static bool scc_svec_grow(void *restrict *svec, size_t capacity, size_t elemsize);

static inline bool scc_svec_is_allocd(void const *svec) {
    return ((unsigned char const*)svec)[-1];
}

static inline size_t scc_svec_memsize(size_t capacity, size_t elemsize, size_t npad) {
    return capacity * elemsize + sizeof(struct scc_svec) + npad;
}

static inline size_t scc_svec_calc_new_capacity(size_t current) {
    return current + (current < SCC_SVEC_MAX_CAPACITY_INCREASE ?
                                current : SCC_SVEC_MAX_CAPACITY_INCREASE);
}


static struct scc_svec *scc_svec_alloc(size_t nbytes, size_t nelems, size_t npad) {
    struct scc_svec *v = malloc(nbytes);
    if(!v) {
        return 0;
    }
    v->sc_size = nelems;
    v->sc_buffer[npad - 2u] = npad - 2 * sizeof(unsigned char);
    v->sc_buffer[npad - 1u] = 1u;
    return v;
}

static bool scc_svec_grow(void *restrict *svec, size_t capacity, size_t elemsize) {
    struct scc_svec *v;
    size_t const npad = scc_svec_impl_npad(*svec);
    size_t const nbytes = scc_svec_memsize(capacity, elemsize, npad);

    if(!scc_svec_is_allocd(*svec)) {
        v = scc_svec_alloc(nbytes, scc_svec_size(*svec), npad);
        if(!v) {
            return false;
        }
        memcpy(v->sc_buffer + npad, *svec, scc_svec_size(*svec) * elemsize);
    }
    else {
        v = realloc(scc_svec_impl_base(*svec), nbytes);
        if(!v) {
            return false;
        }
    }

    v->sc_capacity = capacity;
    *svec = v->sc_buffer + npad;
    return true;
}

void *scc_svec_impl_init(void *initvec, size_t offset, size_t capacity) {
    struct scc_svec *svec = initvec;
    svec->sc_capacity = capacity;
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
    scc_svec_impl_base(vec)->sc_size = size;
    return vec;
}

void scc_svec_free(void *svec) {
    if(scc_svec_is_allocd(svec)) {
        free(scc_svec_impl_base(svec));
    }
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

