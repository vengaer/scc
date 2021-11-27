#include <scc/scc_arena.h>

#include <stdlib.h>

void scc_arena_release(struct scc_arena *arena) {
    struct scc_chunk *iter, *hare;
    scc_arena_foreach_chunk_safe(iter, hare, arena) {
        free(iter);
    }
}
