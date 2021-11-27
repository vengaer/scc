#include <scc/scc_arena.h>

#include <unity.h>

void test_scc_arena_alloc_single_chunk(void) {
    struct scc_arena arena = scc_arena_init(int);
    /* No memory should have been allocated yet */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);
    TEST_ASSERT_EQUAL_PTR(0, arena.ar_current);
    /* Allocate single element */
    int *elem0 = scc_arena_alloc(&arena);
    TEST_ASSERT_EQUAL_UINT32(1u, arena.ar_current->ch_refcount);
    /* Single chunk expected in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);

    int *elem1 = scc_arena_alloc(&arena);
    /* Elements should be contiguous */
    TEST_ASSERT_EQUAL_PTR(elem1, (unsigned char *)elem0 + sizeof(*elem0));
    TEST_ASSERT_EQUAL_UINT32(2u, arena.ar_current->ch_refcount);
    /* Only a single chunk in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);

    int *elem2 = scc_arena_alloc(&arena);
    /* Contiguous elements */
    TEST_ASSERT_EQUAL_PTR(elem2, (unsigned char *)elem1 + sizeof(*elem1));
    TEST_ASSERT_EQUAL_UINT32(3u, arena.ar_current->ch_refcount);
    /* Single chunk in arena */
    TEST_ASSERT_EQUAL_PTR(arena.ar_first, arena.ar_current);

    scc_arena_release(&arena);
}

void test_scc_arena_alloc_multiple_chunks(void) {
    struct scc_arena arena = scc_arena_init(int);

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
    struct scc_arena arena = scc_arena_init(int);
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
    struct scc_arena arena = scc_arena_init(int);
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
