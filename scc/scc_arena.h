#ifndef SCC_ARENA_H
#define SCC_ARENA_H

#include "scc_mem.h"

#include <stddef.h>

#ifndef SCC_ARENA_CHUNKSIZE
enum { SCC_ARENA_CHUNKSIZE = 256 };
#endif

struct scc_arena {
    struct scc_chunk *ar_first;     /* First chunk */
    struct scc_chunk *ar_current;   /* Current (last) chunk */
    unsigned short ar_baseoff;      /* Buffer offset in chunk */
    unsigned short ar_elemsize;     /* Size of element in chunk buffer */
    unsigned short ar_chunksize;    /* Number of elements in each chunk */
};

struct scc_chunk {
    unsigned ch_refcount;           /* Number of non-freed elements in this chunk */
    unsigned ch_offset;             /* Offset of next element relative address of chunk */
    unsigned ch_end;                /* Offset of last element relative address of chunk */
    struct scc_chunk *ch_next;      /* Next chunk */
    unsigned char ch_buffer[];
};

#define scc_chunk_impl_layout(type)                                 \
    struct {                                                        \
        unsigned ch_refcount;                                       \
        unsigned ch_offset;                                         \
        unsigned ch_end;                                            \
        struct scc_chunk *ch_next;                                  \
        type ch_buffer[];                                           \
    }

#define scc_arena_impl_baseoff(type)                                \
    offsetof(scc_chunk_impl_layout(type), ch_buffer)

#define scc_arena_init(type)                                        \
    (struct scc_arena){                                             \
        .ar_baseoff = scc_arena_impl_baseoff(type),                 \
        .ar_elemsize = sizeof(type),                                \
        .ar_chunksize = SCC_ARENA_CHUNKSIZE                         \
    }

void scc_arena_release(struct scc_arena *arena);

#define scc_arena_foreach_chunk(iter, arena)                        \
    for(iter = arena->ar_first; iter; iter = iter->ch_next)

#define scc_arena_foreach_chunk_safe(iter, hare, arena)             \
    for(iter = arena->ar_first, hare = iter ? iter->ch_next : 0;    \
        iter;                                                       \
        iter = hare, hare = hare ? hare->ch_next : 0)

#define scc_arena_foreach_chunk_lagging(iter, tortoise, arena)      \
    for(iter = arena->ar_first, tortoise = 0;                       \
        iter;                                                       \
        tortoise = iter, iter = iter->ch_next)

void *scc_arena_alloc(struct scc_arena *arena);
void scc_arena_free(struct scc_arena *restrict arena, void const *restrict addr);

#endif /* SCC_ARENA_H */
