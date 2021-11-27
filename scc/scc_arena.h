#ifndef SCC_ARENA_H
#define SCC_ARENA_H

struct scc_arena {
    struct scc_chunk *ar_first;
    struct scc_chunk *ar_current;
};

struct scc_chunk {
    unsigned ch_refcount;
    unsigned ch_offset;
    struct scc_chunk *ch_next;
    unsigned char ch_buffer[];
};

#define scc_arena_init() (struct scc_arena){ 0 };
void scc_arena_release(struct scc_arena *arena);

#define scc_arena_foreach_chunk(iter, arena)            \
    for(iter = arena->ar_first; iter <= arena->ar_current; iter = iter->ch_next)

#define scc_arena_foreach_chunk_safe(iter, hare, arena) \
    for(iter = arena->ar_first, hare = iter->ch_next;   \
        iter <= arena->ar_current;                      \
        iter = hare, hare = hare ? hare->ch_next : 0)

#endif /* SCC_ARENA_H */
