/* Clone a btmap */

#include <scc/btmap.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

/* Comparator adheres to the pattern used by strcmp,
 * memcmp, etc., i.e.
 *
 * - Negative return value implies that *l < *r
 * - Positive return value implies that *l > *r
 * - Return value 0 implies that *l == *r
 */
static int compare_int(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

int main(void) {
    /* Instantiate a map with order 32 */
    scc_btmap(int, int) btmap = scc_btmap_with_order(int, int, compare_int, 32);

    /* Insert a couple of values */
    for (int i = 0; i < 12; ++i)
        assert(scc_btmap_insert(&btmap, i, 2 * i));

    /* Create an exact copy of the btmap */
    scc_btmap(int, int) copy = scc_btmap_clone(btmap);

    /* Sizes of the two maps are the same */
    printf("Size of original: %zu, copy: %zu\n", scc_btmap_size(btmap),
        scc_btmap_size(copy));
    assert(scc_btmap_size(btmap) == scc_btmap_size(copy));

    /* Copy contains the same key-value pairs */
    int *old, *new;
    for (int i = 0; i < (int)scc_btmap_size(btmap); ++i) {
        old = scc_btmap_find(btmap, i);
        new = scc_btmap_find(copy, i);

        assert(old && new);
        assert(*old == *new);
        printf("Key: %d, old: %d, new: %d\n", i, *old, *new);
    }

    /* Free the map */
    scc_btmap_free(btmap);
}

/* ============= OUTPUT =============== */
// STDOUT:Size of original: 12, copy: 12
// STDOUT:Key: 0, old: 0, new: 0
// STDOUT:Key: 1, old: 2, new: 2
// STDOUT:Key: 2, old: 4, new: 4
// STDOUT:Key: 3, old: 6, new: 6
// STDOUT:Key: 4, old: 8, new: 8
// STDOUT:Key: 5, old: 10, new: 10
// STDOUT:Key: 6, old: 12, new: 12
// STDOUT:Key: 7, old: 14, new: 14
// STDOUT:Key: 8, old: 16, new: 16
// STDOUT:Key: 9, old: 18, new: 18
// STDOUT:Key: 10, old: 20, new: 20
// STDOUT:Key: 11, old: 22, new: 22
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

