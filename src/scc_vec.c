#include <scc/scc_panic.h>
#include <scc/scc_vec.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { SCC_VEC_MAX_CAPACITY_INCREASE = 4096 };

size_t scc_vec_size(void const *vec);
size_t scc_vec_capacity(void const *vec);
void scc_vec_pop(void *vec);
bool scc_vec_empty(void const *vec);
void scc_vec_clear(void *vec);

static size_t scc_vec_memsize(size_t capacity, size_t elemsize);
static size_t scc_vec_calc_new_capacity(size_t current);
static bool scc_vec_grow(void **vec, size_t capacity, size_t elemsize);
static struct scc_vec *scc_vec_base(void *vec);

static void *scc_vec_null = (union {
    struct scc_vec vec;
    unsigned char bytes[sizeof(struct scc_vec)];
}){ 0 }.vec.sc_buffer;

static inline size_t scc_vec_memsize(size_t capacity, size_t elemsize) {
    return capacity * elemsize + sizeof(struct scc_vec);
}

static inline size_t scc_vec_calc_new_capacity(size_t current) {
    return current + (current < SCC_VEC_MAX_CAPACITY_INCREASE ?
                                (current | 1) : SCC_VEC_MAX_CAPACITY_INCREASE);
}

static inline struct scc_vec *scc_vec_base(void *vec) {
    return scc_container(vec, struct scc_vec, sc_buffer);
}

static bool scc_vec_grow(void **vec, size_t capacity, size_t elemsize) {
    struct scc_vec *v;
    size_t nbytes = scc_vec_memsize(capacity, elemsize);
    if(!scc_vec_capacity(*vec)) {
        v = malloc(nbytes);

        if(!v) {
            return false;
        }
        v->sc_size = 0;
    }
    else {
        v = realloc(scc_vec_base(*vec), nbytes);
        if(!v) {
            return false;
        }
    }

    v->sc_capacity = capacity;
    *vec = v->sc_buffer;
    return true;
}

void *scc_vec_init(void) {
    return scc_vec_null;
}

void scc_vec_free(void *vec) {
    if(scc_vec_capacity(vec)) {
        free(scc_vec_base(vec));
    }
}

bool scc_vec_impl_resize(void *vec, size_t size, size_t elemsize) {
    size_t currsize = scc_vec_size(*(void **)vec);

    if(!size || currsize == size) {
        return true;
    }
    if(currsize > size) {
        scc_vec_base(*(void **)vec)->sc_size = size;
        return true;
    }

    if(!scc_vec_impl_reserve(vec, size, elemsize)) {
        return false;
    }

    unsigned char *baseaddr = *(void **)vec;
    size_t zsize = (size - currsize) * elemsize;

    memset(baseaddr + currsize * elemsize, 0, zsize);
    scc_vec_base(*(void **)vec)->sc_size = size;
    return true;
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

void scc_vec_impl_erase(void *vec, void *iter, size_t elemsize) {
    size_t offset = ((unsigned char *)iter - (unsigned char *)vec) / elemsize;
    --scc_vec_base(vec)->sc_size;
    if(offset == scc_vec_size(vec)) {
        /* Last element */
        return;
    }
    size_t nbytes = (scc_vec_size(vec) - offset) * elemsize;
    memmove(iter, (unsigned char *)iter + elemsize, nbytes);
}

void scc_vec_impl_erase_range(void *vec, void *first, void *end, size_t elemsize) {
    if(end <= first) {
        return;
    }

    size_t nelems = ((unsigned char *)end - (unsigned char *)first) / elemsize;
    size_t offset = ((unsigned char *)first - (unsigned char *)vec) / elemsize;
    scc_vec_base(vec)->sc_size -= nelems;

    if(offset == scc_vec_size(vec)) {
        /* Last nelems elements */
        return;
    }
    size_t nbytes = (scc_vec_size(vec) - offset) * elemsize;
    memmove(first, (unsigned char *)first + nelems * elemsize, nbytes);
}

void scc_vec_pop_safe(void *vec) {
    if(!scc_vec_size(vec)) {
        scc_panic("Attempt to pop element from vector of size 0");
    }
    scc_vec_pop(vec);
}

void scc_vec_impl_at_check(void *vec, size_t index) {
    if(scc_vec_size(vec) <= index) {
        scc_panic("Attempt to access element %zu in vector of size %zu", index, scc_vec_size(vec));
    }
}
