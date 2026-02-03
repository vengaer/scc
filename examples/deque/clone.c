/* Copy a deque */

#include <scc/deque.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create the original deque */
    scc_deque(int) deque = scc_deque_new(int);

    /* Push 32 values */
    for (int i = 0; i < 32; ++i)
        assert(scc_deque_push_back(&deque, i * 2));

    /* Original contains 32 values */
    printf("Size after pushing: %zu\n", scc_deque_size(deque));

    /* Create a copy */
    scc_deque(int) copy = scc_deque_clone(deque);

    /* NULL is returned on failure */
    assert(copy);

    /* Copy contains 32 values */
    printf("Size of copy: %zu\n", scc_deque_size(copy));

    /* Free the original instance */
    scc_deque_free(deque);

    /* Free the copy */
    scc_deque_free(copy);
}

/* ============= OUTPUT =============== */
// STDOUT:Size after pushing: 32
// STDOUT:Size of copy: 32
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
