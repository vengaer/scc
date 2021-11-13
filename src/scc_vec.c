#include <scc/scc_vec.h>

#include <stdbool.h>
#include <stdlib.h>

enum { SCC_VEC_MAX_CAPACITY_INCREASE = 4096 };

size_t scc_vec_size(void const *vec);
size_t scc_vec_capacity(void const *vec);
void scc_vec_pop(void *vec);

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
