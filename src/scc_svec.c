#include <scc/scc_svec.h>

#include <stdbool.h>
#include <stdlib.h>

size_t scc_svec_impl_npad(void const *svec);

static bool scc_svec_is_allocd(void const *svec);

static inline bool scc_svec_is_allocd(void const *svec) {
    return ((unsigned char const*)svec)[-1];
}

void *scc_svec_impl_init(void *initvec, size_t offset, size_t capacity) {
    struct scc_svec *svec = initvec;
    svec->sc_capacity = capacity;
    unsigned char *buffer = (unsigned char *)initvec + offset;
    buffer[-2] = offset - sizeof(*svec) - 2 * sizeof(*buffer);
    return buffer;
}

void scc_svec_free(void *svec) {
    if(scc_svec_is_allocd(svec)) {
        free(scc_svec_impl_base(svec));
    }
}
