/* Initialize a vector at instantiation */

#include <scc/vec.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create an int vec containing the 1, 2, 28 and 333 */
    scc_vec(int) vec = scc_vec_from(int, 1, 2, 28, 333);

    /* May have failed */
    assert(vec);

    /* Print the values */
    for (unsigned i = 0u; i < scc_vec_size(vec); ++i)
        printf("%u: %d\n", i, vec[i]);

    /* Free the vec */
    scc_vec_free(vec);
}

/* ============= OUTPUT =============== */
// STDOUT:0: 1
// STDOUT:1: 2
// STDOUT:2: 28
// STDOUT:3: 333
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
