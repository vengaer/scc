#ifndef SCC_VEC_H
#define SCC_VEC_H

#include <stddef.h>

struct scc_vec {
    size_t sc_size;
    size_t sc_capacity;
    unsigned char sc_buffer[];
};

void *scc_vec_init(void);

#endif /* SCC_VEC_H */
