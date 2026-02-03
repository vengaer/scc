/* Example btree cloning */

#include <scc/btree.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#error assert does nothing
#endif

/*
 * Int comparator function
 */
static int compare_int(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

int main(void) {
    extern int compare_int(void const *l, void const *r);

    scc_btree(int) btree = scc_btree_new(int, compare_int);

    _Bool inserted = true;
    for (int i = 0; i < 12; ++i)
        inserted &= scc_btree_insert(&btree, i);
    assert(inserted);

    /* Create an exact copy of the btree */
    scc_btree(int) copy = scc_btree_clone(btree);

    /* Allocation may fail */
    assert(copy);

    assert(scc_btree_size(btree) == scc_btree_size(copy));

    /* Copy contains the same values */
    int const *old, *new;
    for(int i = 0; i < (int)scc_btree_size(btree); ++i) {
        old = scc_btree_find(btree, i);
        new = scc_btree_find(copy, i);
        assert(old && new);
        printf("Old: %d, new: %d\n", *old, *new);
    }

    scc_btree_free(btree);
    /* The copy must be freed too */
    scc_btree_free(copy);
}

/* ============= OUTPUT =============== */
// STDOUT:Old: 0, new: 0
// STDOUT:Old: 1, new: 1
// STDOUT:Old: 2, new: 2
// STDOUT:Old: 3, new: 3
// STDOUT:Old: 4, new: 4
// STDOUT:Old: 5, new: 5
// STDOUT:Old: 6, new: 6
// STDOUT:Old: 7, new: 7
// STDOUT:Old: 8, new: 8
// STDOUT:Old: 9, new: 9
// STDOUT:Old: 10, new: 10
// STDOUT:Old: 11, new: 11
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
