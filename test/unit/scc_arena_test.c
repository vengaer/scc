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
