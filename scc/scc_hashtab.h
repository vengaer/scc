#ifndef SCC_HASHTAB_H
#define SCC_HASHTAB_H

#include <stddef.h>

enum { SCC_HASHTAB_STACKCAP = 32 };

typedef _Bool(*scc_eq)(void const *, void const *);

#define scc_hashtab(type) type *

struct scc_hashtab {
    scc_eq ht_eq;
    size_t ht_mdoff;
    size_t ht_size;
    size_t ht_capacity;
    unsigned char ht_dynalloc;
    unsigned char ht_fwoff;
    unsigned char ht_buffer[];
};

#define scc_hashtab_impl_layout(type)                               \
    struct {                                                        \
        scc_eq ht_eq;                                               \
        size_t ht_mdoff;                                            \
        size_t ht_size;                                             \
        size_t ht_capacity;                                         \
        unsigned char ht_dynalloc;                                  \
        unsigned char ht_fwoff;                                     \
        unsigned char ht_bkoff;                                     \
        type ht_tmp;                                                \
        type ht_data[SCC_HASHTAB_STACKCAP];                         \
        unsigned short ht_hash[SCC_HASHTAB_STACKCAP];               \
        unsigned char ht_hash_guard[SCC_VECSIZE - 1u];              \
    }

#define scc_hashtab_impl_initsize(type)                             \
    sizeof(scc_hashtab_impl_layout(type))

#define scc_hashtab_impl_inittab(type)                              \
    (union {                                                        \
        struct scc_hashtab sc_tab;                                  \
        unsigned char sc_buffer[scc_hashtab_impl_initsize(type)];   \
    }){ 0 }.sc_tab

#define scc_hashtab_impl_dataoff(type)                              \
    offsetof(scc_hashtab_impl_layout(type), ht_tmp)

#define scc_hashtab_impl_mdoff(type)                                \
    offsetof(scc_hashtab_impl_layout(type), ht_hash)

#define scc_hashtab_init(type, eq)                                  \
    scc_hashtab_impl_init(                                          \
        &scc_hashtab_impl_inittab(type),                            \
        eq,                                                         \
        scc_hashtab_impl_dataoff(type),                             \
        scc_hashtab_impl_mdoff(type),                               \
        SCC_HASHTAB_STACKCAP                                        \
    )

void *scc_hashtab_impl_init(void *inittab, scc_eq eq, size_t dataoff, size_t mdoff, size_t capacity);

#endif /* SCC_HASHTAB_H */
