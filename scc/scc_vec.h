#ifndef SCC_VEC_H
#define SCC_VEC_H

#include "scc_macro.h"

#include <stddef.h>

struct scc_vec {
    size_t sc_size;
    size_t sc_capacity;
    unsigned char sc_buffer[];
};

void *scc_vec_init(void);
void scc_vec_free(void *vec);

inline size_t scc_vec_size(void const *vec) {
    return scc_container_qual(vec, struct scc_vec, sc_buffer, const)->sc_size;
}

inline size_t scc_vec_capacity(void const *vec) {
    return scc_container_qual(vec, struct scc_vec, sc_buffer, const)->sc_capacity;
}

#endif /* SCC_VEC_H */
