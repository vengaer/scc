#include <scc/arena.h>
#include <scc/scc_dbg.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

static struct scc_chunk *scc_chunk_new(size_t chunksize, size_t elemsize, size_t baseoff);
static bool scc_chunk_contains_addr(struct scc_chunk const *chunk, void const *addr);

static struct scc_chunk *scc_chunk_new(size_t chunksize, size_t elemsize, size_t baseoff) {
    size_t const size = chunksize * elemsize + baseoff;
    struct scc_chunk *chunk = malloc(size);
    if(!chunk) {
        return 0;
    }
    /* Assume chunk is to be used immediately */
    chunk->ch_refcount = 1;
    /* Offset of second element */
    chunk->ch_offset = baseoff;
    /* Offset of element after last in chunk */
    chunk->ch_end = chunk->ch_offset - elemsize + chunksize * elemsize;
    chunk->ch_next = 0;
    return chunk;
}

static inline bool scc_chunk_contains_addr(struct scc_chunk const *chunk, void const *addr) {
    return addr <= (void const *)((unsigned char const *)chunk + chunk->ch_end) &&
           (void const *)chunk < addr;
}

void scc_arena_release(struct scc_arena *arena) {
    struct scc_chunk *iter;
    struct scc_chunk *hare;
    scc_arena_foreach_chunk_safe(iter, hare, arena) {
        free(iter);
    }
}

void *scc_arena_alloc(struct scc_arena *arena) {
    size_t const chunksize = arena->ar_chunksize;
    size_t const elemsize = arena->ar_elemsize;
    size_t const baseoff = arena->ar_baseoff;
    struct scc_chunk *chunk;

    if(!arena->ar_current) {
        /* No chunks in arena */
        chunk = scc_chunk_new(chunksize, elemsize, baseoff);
        if(!chunk) {
            return 0;
        }

        arena->ar_current = chunk;
        arena->ar_first = chunk;
    }
    else if(arena->ar_current->ch_offset == arena->ar_current->ch_end) {
        /* Chunk full */
        chunk = scc_chunk_new(chunksize, elemsize, baseoff);
        if(!chunk) {
            return 0;
        }
        arena->ar_current->ch_next = chunk;
        arena->ar_current = chunk;
    }
    else {
        chunk = arena->ar_current;
        ++chunk->ch_refcount;
        chunk->ch_offset += elemsize;
    }

    return (unsigned char *)chunk + chunk->ch_offset;
}

_Bool scc_arena_reserve(struct scc_arena *arena, size_t nelems) {
    if(arena->ar_current && arena->ar_current->ch_end - arena->ar_current->ch_offset >= nelems * arena->ar_elemsize) {
        /* Enough space in chunk */
        return true;
    }

    size_t chunksize = nelems < arena->ar_chunksize ? arena->ar_chunksize : nelems;
    struct scc_chunk *chunk = scc_chunk_new(chunksize, arena->ar_elemsize, arena->ar_baseoff);
    if(!chunk) {
        return false;
    }

    if(!arena->ar_current) {
        arena->ar_current = chunk;
        arena->ar_first = chunk;
    }
    else {
        arena->ar_current->ch_next = chunk;
        arena->ar_current = chunk;
    }

    return true;
}

_Bool scc_arena_try_free(struct scc_arena *restrict arena, void const *restrict addr) {
    struct scc_chunk *iter;
    struct scc_chunk *tortoise;
    scc_arena_foreach_chunk_lagging(iter, tortoise, arena) {
        if(scc_chunk_contains_addr(iter, addr)) {
            break;
        }
    }
    if(!iter) {
        return false;
    }
    --iter->ch_refcount;
    if(iter->ch_offset == iter->ch_end && !iter->ch_refcount) {
        if(!tortoise) {
            /* First chunk */
            arena->ar_first = iter->ch_next;
        }
        else {
            tortoise->ch_next = iter->ch_next;
        }
        if(iter == arena->ar_current) {
            arena->ar_current = tortoise;
        }
        free(iter);
    }
    return true;
}

void scc_arena_free(struct scc_arena *restrict arena, void const *restrict addr) {
    scc_bug_on(!scc_arena_try_free(arena, addr));
}
