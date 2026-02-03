/* Example btree insertion */

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
    for (int i = 0; i < 5; ++i)
        inserted &= scc_btree_insert(&btree, i);
    assert(inserted);

    for (unsigned i = 0; i < scc_btree_size(btree); ++i)
        printf("Lookup %u: %d\n", i, *(int *)scc_btree_find(btree, i));

    scc_btree_free(btree);
}

/* ============= OUTPUT =============== */
// STDOUT:Lookup 0: 0
// STDOUT:Lookup 1: 1
// STDOUT:Lookup 2: 2
// STDOUT:Lookup 3: 3
// STDOUT:Lookup 4: 4
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
