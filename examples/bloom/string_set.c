/* Storing strings in a bloom filter */

#include <scc/bloom.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void hash_string(struct scc_digest128 *digest, void const *data,
                size_t sz, uint_fast32_t seed) {
    /* Default hash function uses size of the element whose address is
     * passed in the data parameter. Want the length of the string
     * instead. */
    (void)sz;
    scc_hash_murmur128(digest, data, strlen(data), seed);
}

int main(void) {
    /* Create a 128 bit bloom filter storing char const * instances and using
     * 8 hash functions derived from hash_string defined above */
    scc_bloom(char const *) flt = scc_bloom_with_hash(char const *, 128u, 8u, hash_string);

    /* Insert a couple of strings */
    scc_bloom_insert(&flt, "Some string");
    scc_bloom_insert(&flt, "Another string");
    scc_bloom_insert(&flt, "Yet another string");
    scc_bloom_insert(&flt, "AAAAAAAAAAAAAAAAAA");

    /* Print properties */
    printf("Filter bit size: %zu\n", scc_bloom_capacity(flt) << 3u);
    printf("Number of hash functions: %zu\n", scc_bloom_nhashes(flt));

    /* Test for presence of a couple of strings */
    printf("\"Some string\" in set: %s\n",
        scc_bloom_test(flt, "Some string") ? "yes" : "no");

    printf("\"Yet another string\" in set: %s\n",
        scc_bloom_test(flt, "Yet another string") ? "yes" : "no");

    printf("\"Conniptions\" in set: %s\n",
        scc_bloom_test(flt, "Conniptions") ? "yes" : "no");

    /* Destroy the instance */
    scc_bloom_free(flt);
}

/* ============= OUTPUT =============== */
// STDOUT:Filter bit size: 128
// STDOUT:Number of hash functions: 8
// STDOUT:"Some string" in set: yes
// STDOUT:"Yet another string" in set: yes
// STDOUT:"Conniptions" in set: no
/* ==================================== */

// RUN: %cc %s %dynamic -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT

// RUN: %cc %s %static -o %t
// RUN: %t | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
