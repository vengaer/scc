#include <scc/scc_vec.h>

#include <stdlib.h>

size_t scc_vec_size(void const *vec);
size_t scc_vec_capacity(void const *vec);

static void *scc_vec_null = (union {
    struct scc_vec vec;
    unsigned char bytes[sizeof(struct scc_vec)];
}){ 0 }.vec.sc_buffer;

static inline struct scc_vec *scc_vec_base(void *vec) {
    return scc_container(vec, struct scc_vec, sc_buffer);
}

void *scc_vec_init(void) {
    return scc_vec_null;
}

void scc_vec_free(void *vec) {
    if(scc_vec_capacity(vec)) {
        free(scc_vec_base(vec));
    }
}
