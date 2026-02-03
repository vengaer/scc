/* Example hashtab insertion */

#include <scc/hashtab.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert has not effect
#endif

/*
 * Int comparator
 */
static _Bool int_eq(void const *l, void const *r) {
    return *(int const *)l == *(int const *)r;
}

int main(void) {
    extern _Bool int_eq(void const *l, void const *r);

    scc_hashtab(int) tab = scc_hashtab_new(int, int_eq);

    /* scc_hashtab_insert takes the pointer to the table handle, not the handle itself */
    _Bool inserted = scc_hashtab_insert(&tab, 12);
    assert(inserted);

    /* Insert another value */
    inserted = scc_hashtab_insert(&tab, 13);
    assert(inserted);

    /* Duplicates are not supported */
    inserted = scc_hashtab_insert(&tab, 12);
    assert(!inserted);

    /* Look up and print values */
    printf("12 is in the table: %s\n", scc_hashtab_find(tab, 12) ? "true" : "false");
    printf("13 is in the table: %s\n", scc_hashtab_find(tab, 13) ? "true" : "false");
    printf("14 is in the table: %s\n", scc_hashtab_find(tab, 14) ? "true" : "false");

    /* Need to free the table */
    scc_hashtab_free(tab);
}

/* ============= OUTPUT =============== */
// STDOUT:12 is in the table: true
// STDOUT:13 is in the table: true
// STDOUT:14 is in the table: false
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
