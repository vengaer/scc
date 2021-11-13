#include <scc/scc_vec.h>

static void *scc_vec_null = (union {
    struct scc_vec vec;
    unsigned char bytes[sizeof(struct scc_vec)];
}){ 0 }.vec.sc_buffer;

void *scc_vec_init(void) {
    return scc_vec_null;
}
