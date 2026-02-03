/* Example rbtree iteration */

#include <scc/rbtree.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert has not effect
#endif

/*
 * Int comparator
 */
static int compare_int(void const *l, void const *r) {
    return *(int const *)l - *(int const *)r;
}

int main(void) {
    extern int compare_int(void const *l, void const *r);

    /* Create an instance containing ints */
    scc_rbtree(int) tree = scc_rbtree_new(int, compare_int);

    /* Insert a couple of pairs */
    _Bool inserted = true;
    for (int i = 0; i < 6; ++i)
        inserted &= scc_rbtree_insert(&tree, i * 3);
    assert(inserted);

    scc_rbtree_iter(int) it;
    unsigned i = 0u;

    /* Iterate over and print each element */
    scc_rbtree_foreach(it, tree)
        printf("%u: %d\n", i++, *it);

    /* Free the instance */
    scc_rbtree_free(tree);
}

/* ============= OUTPUT =============== */
// STDOUT:0: 0
// STDOUT:1: 3
// STDOUT:2: 6
// STDOUT:3: 9
// STDOUT:4: 12
// STDOUT:5: 15
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
