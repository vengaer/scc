/* == hash_dict.c ==
 *
 * This file serves as an example of how to achieve something
 * akin to a python dict using scc's hash map.
 *
 * The example illustrates how to use fixed-size strings
 * to look up integers.
 */

#include <scc/hashmap.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed-size "string" */
typedef struct { char buf[64]; } str64;

/* The eq function is passed the address of two
 * keys for comparison. */
static bool streq(void const *left, void const *right) {
    return !strcmp(((str64 const *)left)->buf, ((str64 const *)right)->buf);
}

int main(void) {
    /* Declare variable for referring to
     * a hash map mapping str64-instances to
     * ints */
    scc_hashmap(str64, int) map;

    /* Initialize the hash map, providing the key type,
     * value type and comparisong function, in order.
     *
     * The call cannot fail
     * */
    map = scc_hashmap_new(str64, int, streq);

    /* Insert the key-value pair { "first key", 88 }.
     *
     * This is comparable to
     *
     * python:
     *
     *   d = {}
     *   d["first key"] = 88
     *
     * c++:
     *
     *   std::map<std::string, int> m;
     *   m["first key"] = 88;
     */
    bool inserted = scc_hashmap_insert(&map, (str64){ "first key" }, 88);

    /* Insertion may fail. The most common case is
     * where the key is already in the table
     */
    if(!inserted) {
        fputs("Could not insert { \"first key\", 88 }\n", stderr);
        exit(EXIT_FAILURE);
    }

    /* The size of the hash  map may be queried using
     * scc_hashmap_size */
    printf("Map size is %zu\n", scc_hashmap_size(map));

    /* Insert another pair */
    inserted = scc_hashmap_insert(&map, (str64){ "second key" }, 92);
    if(!inserted) {
        fputs("Could not insert { \"second key\", 92 }\n", stderr);
        exit(EXIT_FAILURE);
    }

    /* Looking up values is done using the associated key */
    int *value = scc_hashmap_find(map, (str64){ "first key" });

    printf("Value associated with \"first key\" is %d\n", *value);

    /* The mapped value may be modified through the returned pointer */
    *value = 32;

    printf("Updated value associated with \"first key\" is %d\n", *(int *)scc_hashmap_find(map, (str64){ "first key" }));

    /* If the key proved for is not in the table,
     * find returns NULL */
    value = scc_hashmap_find(map, (str64){ "third key" });
    printf("\"third key\" is %sin map\n", !!value ? "" : "not ");

    /* Key-value pairs may be removed given the key */
    scc_hashmap_remove(map, (str64){ "first key" });

    value = scc_hashmap_find(map, (str64){ "first key" });
    printf("\"first key\" is %sin map\n", !!value ? "" : "no longer ");

    /* The map must be freed */
    scc_hashmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:Map size is 1
// STDOUT:Value associated with "first key" is 88
// STDOUT:Updated value associated with "first key" is 32
// STDOUT:"third key" is not in map
// STDOUT:"first key" is no longer in map
/* ==================================== */

// RUN: %cc -I %root %s %libscc_a -g -o %litbuild/hash_dict
// RUN: %litbuild/hash_dict | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
