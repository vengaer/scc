/* Example of hashtab iteration */

#include <scc/hash.h>
#include <scc/hashtab.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#error assert has not effect
#endif

/*
 * char const * comparator
 */
static _Bool str_eq(void const *l, void const *r) {
    return !strcmp(l, r);
}

/*
 * Default hash function would hash the pointer rather than the character
 * sequence. This might work for very specific use cases buy is certainly
 * not deterministic. To achieve the latter, make sure to hash the sequence
 * rather than its address.
 */
static uint_fast64_t str_hash(void const *p, size_t sz) {
    /* This is sizeof(char const *), don't care about it */
    (void)sz;

    /* Use the alternative FNV1 implementation provided by scc */
    return scc_hash_fnv1a_64(*(char const **)p, strlen(p));
}

int main(void) {
    extern _Bool str_eq(void const *l, void const *r);

    /* If building for a 32-bit system, you may use uint_fast32_t instead */
    extern uint_fast64_t str_hash(void const *p, size_t sz);

    scc_hashtab(char const *) tab = scc_hashtab_with_hash(char const *, str_eq, str_hash);

    char const *const strs[] = {
        "one", "two", "three", "four", "five",
    };

    /* Insert a bunch of strings */
    _Bool inserted = true;
    for (unsigned i = 0u; i < sizeof(strs) / sizeof(strs[0]); ++i)
        inserted &= scc_hashtab_insert(&tab, strs[i]);
    assert(inserted);

    /* Iterator instance */
    scc_hashtab_iter(char const *) it;

    /* it visits each value in the table one after the other */
    scc_hashtab_foreach(it, tab)
        printf("%s\n", *it);

    /* Need to free the table */
    scc_hashtab_free(tab);
}

/* ============= OUTPUT =============== */
// STDOUT:five
// STDOUT:one
// STDOUT:three
// STDOUT:four
// STDOUT:two
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
