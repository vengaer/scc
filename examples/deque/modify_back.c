/* Modify the value at the end of the deque */

#include <scc/deque.h>

#include <assert.h>
#include <stdio.h>

#ifdef NDEBUG
#error assert does nothing
#endif

int main(void) {
    /* Create a deque */
    scc_deque(int) deque = scc_deque_new(int);

    /* Push three values to it */
    assert(scc_deque_push_back(&deque, 1));
    assert(scc_deque_push_back(&deque, 2));
    assert(scc_deque_push_back(&deque, 3));

    /* The value at the end should be 3 */
    assert(scc_deque_back(deque) == 3);
    printf("Value at the end is %d\n", scc_deque_back(deque));

    /* Get a pointer to the last element
     *
     * N.B. pushing a value to the deque may
     * invalidate this pointer
     */
    int const *p = &scc_deque_back(deque);

    /* Modify the value at the end */
    scc_deque_back(deque) = 88;

    /* The value at the end has now changed */
    printf("Updated value at the end is %d\n", scc_deque_back(deque));
    assert(scc_deque_back(deque) == 88);

    /* The pointer refers to the just-changed value */
    assert(*p == 88);
    printf("Dereferenced pointer is %d\n", *p);

    /* Free the instance */
    scc_deque_free(deque);
}

/* ============= OUTPUT =============== */
// STDOUT:Value at the end is 3
// STDOUT:Updated value at the end is 88
// STDOUT:Dereferenced pointer is 88
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
