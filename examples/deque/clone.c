/* Copy a deque */

#include <scc/ringdeque.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create the original deque */
    scc_ringdeque(int) deque = scc_ringdeque_new(int);

    /* Push 32 values */
    for (int i = 0; i < 32; ++i)
        assert(scc_ringdeque_push_back(&deque, i * 2));

    /* Original contains 32 values */
    printf("Size after pushing: %zu\n", scc_ringdeque_size(deque));

    /* Create a copy */
    scc_ringdeque(int) copy = scc_ringdeque_clone(deque);

    /* NULL is returned on failure */
    assert(copy);

    /* Copy contains 32 values */
    printf("Size of copy: %zu\n", scc_ringdeque_size(copy));

    /* Free the original instance */
    scc_ringdeque_free(deque);

    /* Free the copy */
    scc_ringdeque_free(copy);
}

/* ============= OUTPUT =============== */
// STDOUT:Size after pushing: 32
// STDOUT:Size of copy: 32
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
