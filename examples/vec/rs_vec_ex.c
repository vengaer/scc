/* == rs_vec_ex ==
 *
 * vec example from https://doc.rust-lang.org/std/vec/struct.Vec.html
 * translated to C with the help of scc
 *
 * The example looks as follows
 *
 * let mut vec = Vec::new();
 * vec.push(1);
 * vec.push(2);
 *
 * assert_eq!(vec.len(), 2);
 * assert_eq!(vec[0], 1);
 *
 * assert_eq!(vec.pop(), Some(2));
 * assert_eq!(vec.len(), 1);
 *
 * vec[0] = 7;
 * assert_eq!(vec[0], 7);
 *
 * vec.extend([1, 2, 3]);
 *
 * for x in &vec {
 *      println!("{x}");
 * }
 * assert_eq!(vec, [7, 1, 2, 3]);
 */

#include <scc/vec.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#error assert has no effect
#endif

int main(void) {
    /* Initialize a vector of ints */
    scc_vec(int) vec = scc_vec_new(int);

    /* Push some values */
    scc_vec_push(&vec, 1);
    scc_vec_push(&vec, 2);

    /* Size queried with scc_vec_size */
    assert(scc_vec_size(vec) == 2u);
    /* Individual elements accessed as if
     * normal array */
    assert(vec[0] == 1);

    /* Pop the last value and check size */
    assert(scc_vec_pop(vec) == 2);
    assert(scc_vec_size(vec) == 1u);

    /* Values may be overwritten using
     * normal indexing */
    vec[0] = 7;
    assert(vec[0] == 7);

    /* scc_vec has no functionality directly matching
     * rust's vec.extend. Use resize + memcpy */
    scc_vec_resize(&vec, scc_vec_size(vec) + 3u);
    memcpy(&vec[1], (int[]){ 1, 2, 3 }, sizeof(int[3]));

    /* Iterate over and print every element */
    int *iter;
    scc_vec_foreach(iter, vec) {
        printf("%d\n", *iter);
    }

    int const expected[] = { 7, 1, 2, 3 };
    assert(!memcmp(vec, expected, sizeof(expected)));
}

/* ============= OUTPUT =============== */
// STDOUT:7
// STDOUT:1
// STDOUT:2
// STDOUT:3
/* ==================================== */

// RUN: %cc -I %root %s %libscc_a -g -o %litbuild/rs_vec_ex
// RUN: %litbuild/rs_vec_ex | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
