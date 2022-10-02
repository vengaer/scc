#include <scc/scc_arena.h>
#include <scc/scc_mem.h>

#include <unity.h>

void test_scc_arena_release_empty(void) {
    struct scc_arena arena = scc_arena_new(int);
    scc_arena_release(&arena);
}

void test_scc_arena_alloc_single_chunk(void) {
    struct scc_arena arena = scc_arena_new(int);
    /* No memory should have been allocated yet */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);
    TEST_ASSERT_EQUAL_PTR(0, arena.ar_current);
    /* Allocate single element */
    int *elem0 = scc_arena_alloc(&arena);
    TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);
    /* Single chunk expected in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);
    (void)elem0;

#if SCC_ARENA_CHUNKSIZE > 1
    int *elem1 = scc_arena_alloc(&arena);
    /* Elements should be contiguous */
    TEST_ASSERT_EQUAL_PTR(elem1, (unsigned char *)elem0 + sizeof(*elem0));
    TEST_ASSERT_EQUAL_UINT32(2u, arena.ar_current->ch_refcount);
    /* Only a single chunk in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);
    (void)elem1;
#endif

#if SCC_ARENA_CHUNKSIZE > 2
    int *elem2 = scc_arena_alloc(&arena);
    /* Contiguous elements */
    TEST_ASSERT_EQUAL_PTR(elem2, (unsigned char *)elem1 + sizeof(*elem1));
    TEST_ASSERT_EQUAL_UINT32(3u, arena.ar_current->ch_refcount);
    /* Single chunk in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);
#endif

    scc_arena_release(&arena);
}

void test_scc_arena_alloc_multiple_chunks(void) {
    struct scc_arena arena = scc_arena_new(int);

    int *prev = scc_arena_alloc(&arena);
    int *curr;
    /* Fill up entire chunk */
    for(unsigned i = 0u; i < arena.ar_chunksize - 1u; i++) {
        curr = scc_arena_alloc(&arena);
        /* Verify refcount */
        TEST_ASSERT_EQUAL_UINT32(i + 2u, arena.ar_current->ch_refcount);
        /* Elements should be contiguous */
        TEST_ASSERT_EQUAL_PTR((unsigned char *)prev + sizeof(*prev), curr);
        /* Single chunk in arena */
        TEST_ASSERT_EQUAL_PTR(0, arena.ar_first->ch_next);
        TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);
        prev = curr;
    }

    /* Push new chunk into arena */
    prev = scc_arena_alloc(&arena);
    /* Refcount of new chunk */
    TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);
    for(unsigned i = 0u; i < arena.ar_chunksize - 1u; i++) {
        curr = scc_arena_alloc(&arena);
        /* Verify refcount */
        TEST_ASSERT_EQUAL_UINT32(i + 2u, arena.ar_current->ch_refcount);
        /* Contiguous addresses */
        TEST_ASSERT_EQUAL_PTR((unsigned char *)prev + sizeof(*prev), curr);
        /* Expect two chunks in arena */
        TEST_ASSERT_EQUAL_PTR(arena.ar_current, arena.ar_first->ch_next);
        /* No chunk following ar_current */
        TEST_ASSERT_EQUAL_PTR(0, arena.ar_current->ch_next);
        prev = curr;
    }

    scc_arena_release(&arena);
}

void test_scc_arena_free_single_chunk(void) {
    struct scc_arena arena = scc_arena_new(int);
    int *elem;
    for(unsigned i = 0; i < arena.ar_chunksize - 1u; i++) {
        /* Allocate and verify refcount */
        elem = scc_arena_alloc(&arena);
        TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);

        /* Free and verify refcount */
        scc_arena_free(&arena, elem);
        TEST_ASSERT_EQUAL_UINT32(0u, arena.ar_current->ch_refcount);
    }
    /* Allocate and free last element in chunk */
    elem = scc_arena_alloc(&arena);
    TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);
    scc_arena_free(&arena, elem);

    /* No chunks in arena */
    TEST_ASSERT_EQUAL_PTR(0, arena.ar_current);
    TEST_ASSERT_EQUAL_PTR(0, arena.ar_first);
    /* No release required */
}

void test_scc_arena_free_multiple_chunks(void) {
    struct scc_arena arena = scc_arena_new(int);
    /* Use up entire first chunk */
    int *first = scc_arena_alloc(&arena);
    for(unsigned i = 0u; i < arena.ar_chunksize - 1u; i++) {
        scc_arena_alloc(&arena);
    }
    /* Chunk should be full */
    TEST_ASSERT_EQUAL_UINT32(arena.ar_chunksize, arena.ar_current->ch_refcount);

    /* First element in second chunk */
    int *second = scc_arena_alloc(&arena);

    struct scc_chunk *chunk = (struct scc_chunk *)((unsigned char *)second - arena.ar_baseoff);

    /* Free elements in first chunk */
    for(unsigned i = 0u; i < arena.ar_chunksize; i++) {
        TEST_ASSERT_EQUAL_UINT32(arena.ar_chunksize - i, arena.ar_first->ch_refcount);
        scc_arena_free(&arena, first + i);
    }
    /* Only second chunk should remain in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_current, arena.ar_first);
    TEST_ASSERT_EQUAL_PTR(arena.ar_current, chunk);

    scc_arena_release(&arena);
}

void test_scc_arena_free_middle_chunks(void) {
    enum { NCHUNKS = 5 };
    struct scc_arena arena = scc_arena_new(int);

    int *chks[NCHUNKS];
    /* Allocate elements corresponding to 5 chunks */
    for(unsigned i = 0u; i < scc_arrsize(chks); i++) {
        /* First element of chunk i in chks[i] */
        chks[i] = scc_arena_alloc(&arena);
        /* Verify refcount */
        TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);
        for(unsigned j = 0u; j < arena.ar_chunksize - 1u; j++) {
            /* Throw away remaining allocations */
            scc_arena_alloc(&arena);
        }
    }

    struct scc_chunk *chunk;
    struct scc_chunk *iter = arena.ar_first;
    /* Verify that the respective elements were allocated in the
     * expected chunks */
    for(unsigned i = 0u; i < scc_arrsize(chks); i++) {
        chunk = (struct scc_chunk *)((unsigned char *)chks[i] - arena.ar_baseoff);
        TEST_ASSERT_EQUAL_PTR(chunk, iter);
        TEST_ASSERT_EQUAL_UINT32(arena.ar_chunksize, chunk->ch_refcount);
        iter = iter->ch_next;
    }

    /* Free all elements in chunks 1 and 2 */
    for(unsigned i = 1u; i < 3u; i++) {
        for(unsigned j = 0u; j < arena.ar_chunksize; j++) {
            chunk = (struct scc_chunk *)((unsigned char *)chks[i] - arena.ar_baseoff);
            /* Verify refcount */
            TEST_ASSERT_EQUAL_UINT32(arena.ar_chunksize - j, chunk->ch_refcount);
            scc_arena_free(&arena, chks[i] + j);
        }
    }

    /* First chunk should be origin chunk 0 */
    TEST_ASSERT_EQUAL_PTR((struct scc_chunk *)((unsigned char *)chks[0] - arena.ar_baseoff), arena.ar_first);
    /* Second chunk should be original chunk 3 */
    TEST_ASSERT_EQUAL_PTR((struct scc_chunk *)((unsigned char *)chks[3] - arena.ar_baseoff), arena.ar_first->ch_next);
    /* Third chunk should be original chunk 4 */
    TEST_ASSERT_EQUAL_PTR((struct scc_chunk *)((unsigned char *)chks[4] - arena.ar_baseoff), arena.ar_first->ch_next->ch_next);
    /* Verify list */
    TEST_ASSERT_EQUAL_PTR(arena.ar_current, arena.ar_first->ch_next->ch_next);
    /* Verify end of list */
    TEST_ASSERT_EQUAL_PTR(0, arena.ar_first->ch_next->ch_next->ch_next);

    scc_arena_release(&arena);
}

void test_scc_arena_free_first_chunk(void) {
    enum { BUFLEN = 32 };
    struct foo {
        char c[BUFLEN];
        long long lld;
    };

    struct scc_arena arena = scc_arena_new(struct foo);
    /* Allocate 4 chunks' worth of elements */
    struct foo *first = scc_arena_alloc(&arena);
    for(unsigned i = 0; i < arena.ar_chunksize * 4u - 1u; i++) {
        scc_arena_alloc(&arena);
    }

    struct scc_chunk *chunk = (struct scc_chunk *)((unsigned char *)first - arena.ar_baseoff);
    /* Verify head pointer */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, chunk);
    struct scc_chunk *next = chunk->ch_next;
    /* Free all elements in first chunk */
    for(unsigned i = 0; i < arena.ar_chunksize; i++) {
        TEST_ASSERT_EQUAL_UINT32(arena.ar_chunksize - i, chunk->ch_refcount);
        scc_arena_free(&arena, first + i);
    }
    /* First chunk should no longer be in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, next);

    scc_arena_release(&arena);
}

void test_scc_arena_free_last_chunk(void) {
    struct scc_arena arena = scc_arena_new(int);
    /* Push 4 chunks to arena */
    for(unsigned i = 0u; i < arena.ar_chunksize * 4u; i++) {
        scc_arena_alloc(&arena);
    }
    /* Check list */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first->ch_next->ch_next->ch_next, arena.ar_current);

    /* Current last chunk */
    struct scc_chunk *chunk = arena.ar_current;

    /* Push new chunk to arena */
    int *p = scc_arena_alloc(&arena);
    /* Previous last should hold address of new last */
    TEST_ASSERT_EQUAL_PTR(chunk->ch_next, arena.ar_current);
    /* Refcount of last chunk */
    TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);

    /* Current last chunk */
    chunk = (struct scc_chunk *)((unsigned char *)p - arena.ar_baseoff);
#if SCC_ARENA_CHUNKSIZE > 1
    /* Reduce refcount to 0, chunk should not be freed as there is
     * still plenty of memory available in it */
    scc_arena_free(&arena, p);
    /* Check refcount */
    TEST_ASSERT_EQUAL_UINT32(0u, arena.ar_current->ch_refcount);
    /* Chunk not released */
    TEST_ASSERT_EQUAL_PTR(chunk, arena.ar_current);

    /* Allocate and free single element until only one free
     * slot remains in chunk */
    for(unsigned i = 0; i < arena.ar_chunksize - 2u; i++) {
        p = scc_arena_alloc(&arena);
        TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);
        scc_arena_free(&arena, p);
        TEST_ASSERT_EQUAL_UINT32(0u, arena.ar_current->ch_refcount);
        /* Chunk not released */
        TEST_ASSERT_EQUAL_PTR(chunk, arena.ar_current);
    }

    /* Allocate last element in chunk */
    p = scc_arena_alloc(&arena);
    /* Still not released */
    TEST_ASSERT_EQUAL_PTR(chunk, arena.ar_current);
    /* Check list layout to force check for chunk being released to be correct */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first->ch_next->ch_next->ch_next->ch_next, chunk);
    /* Free last element */
    scc_arena_free(&arena, p);
    /* Chunk should now have been released */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first->ch_next->ch_next->ch_next, arena.ar_current);
#endif

    scc_arena_release(&arena);
}

void test_scc_arena_reserve_enough_space_available(void) {
    struct scc_arena arena = scc_arena_new(int);
    TEST_ASSERT_TRUE(scc_arena_reserve(&arena, 10));
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);
    scc_arena_release(&arena);
}

void test_scc_arena_reserve_insufficient_space(void) {
    struct scc_arena arena = scc_arena_new(int);

    TEST_ASSERT_TRUE(scc_arena_alloc(&arena));
    TEST_ASSERT_TRUE(scc_arena_reserve(&arena, arena.ar_chunksize + 10));

    TEST_ASSERT_EQUAL_PTR(arena.ar_first->ch_next, arena.ar_current);

    TEST_ASSERT_GREATER_THAN_UINT64(arena.ar_chunksize, arena.ar_current->ch_end);

    scc_arena_release(&arena);
}
