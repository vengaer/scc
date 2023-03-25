/* == cppreference_map_ex ==
 *
 * This file translates the std::map
 * example at https://en.cppreference.com/w/cpp/container/map
 * to C with the help of scc.
 *
 * For reference, the example looks as follows:
 *
 * #include <iostream>
 * #include <map>
 * #include <string>
 * #include <string_view>
 *
 * void print_map(std::string_view comment, const std::map<std::string, int>& m)
 * {
 *     std::cout << comment;
 *     // iterate using C++17 facilities
 *     for (const auto& [key, value] : m)
 *         std::cout << '[' << key << "] = " << value << "; ";
 *
 *     // C++11 alternative:
 *     //  for (const auto& n : m)
 *     //      std::cout << n.first << " = " << n.second << "; ";
 *     //
 *     // C++98 alternative
 *     //  for (std::map<std::string, int>::const_iterator it = m.begin(); it != m.end(); it++)
 *     //      std::cout << it->first << " = " << it->second << "; ";
 *
 *     std::cout << '\n';
 * }
 *
 * int main()
 * {
 *     // Create a map of three (string, int) pairs
 *     std::map<std::string, int> m{{"CPU", 10}, {"GPU", 15}, {"RAM", 20}};
 *
 *     print_map("1) Initial map: ", m);
 *
 *     m["CPU"] = 25; // update an existing value
 *     m["SSD"] = 30; // insert a new value
 *     print_map("2) Updated map: ", m);
 *
 *     // using operator[] with non-existent key always performs an insert
 *     std::cout << "3) m[UPS] = " << m["UPS"] << '\n';
 *     print_map("4) Updated map: ", m);
 *
 *     m.erase("GPU");
 *     print_map("5) After erase: ", m);
 *
 *     std::erase_if(m, [](const auto& pair){ return pair.second > 25; });
 *     print_map("6) After erase: ", m);
 *     std::cout << "7) m.size() = " << m.size() << '\n';
 *
 *     m.clear();
 *     std::cout << std::boolalpha << "8) Map is empty: " << m.empty() << '\n';
 * }
 */

#include <scc/rbmap.h>
#include <scc/vec.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* The rbmap will use char const * as keys. This does, of course,
 * require that the pointees remain valid for as long as they are in
 * the map. */
static int compare(void const *left, void const *right) {
    return strcmp(left, right);
}

static void print_map(char const *restrict comment, void *restrict in_map) {
    fputs(comment, stdout);

    /* scc_rbmap(key, value) expands to a pointer to an anonymous struct.
    * An unpleasant consequence of this is that an rbmap handle cannot
    * be directly assigned to another. As such, rbmap parameters
    * must be void pointers. Before using them in the function, the parameter
    * must be cast to the appropriate handle type */
    scc_rbmap(char const *, int) map = in_map;

    /* Iterating over the rbmap requires a compatible iterator.
     * As it happens, scc_rbmap_iter expands to just such a type */
    scc_rbmap_iter(char const *, int) iter;

    /* Iterating over the map is suitably achieved using the
     * scc_rbmap_foreach macro */
    scc_rbmap_foreach(iter, map) {
        /* The key and value of each pair in the map are
         * referenced by the iterator members with the same
         * names */
        printf(" [%s] = %d;", iter->key, iter->value);
    }

    puts("");
}

int main(void) {
    /* Declare each key as a variable to make sure they
     * have a fix address. This is required since the
     * (questionable) choice of storing char const * keys
     * instead of the strings themselves.
     *
     * See hashmap/hash_dict.c for an example of how to make the
     * container "own" the strings */
    char const *cpu = "CPU";
    char const *gpu = "GPU";
    char const *ram = "RAM";
    char const *ssd = "SSD";
    char const *ups = "UPS";

    /* Create an rbmap instance mapping char const *s to ints, initializing
     * it with the key type, value type and comparson function */
    scc_rbmap(char const *, int) map = scc_rbmap_new(char const *, int, compare);

    /* scc has nothing resembling C++'s std::initializer_list, each
     * key-value pair must be inserted one by one.
     *
     * Error checking omitted */
    scc_rbmap_insert(&map, cpu, 10) &&
    scc_rbmap_insert(&map, gpu, 15) &&
    scc_rbmap_insert(&map, ram, 20);

    print_map("1) Initial map:", map);

    /* There is no bracket-operator for the rbmap either. scc_rbmap_insert
     * works analogously though.  */
    scc_rbmap_insert(&map, cpu, 25);  /* Update an existing value */
    scc_rbmap_insert(&map, ssd, 30);  /* Insert a new value */
    print_map("2) Updated map:", map);

    /* scc_rbmap does, unfortunately, not have any counterpart
     * to std::map::operator[] inserting default-constructed
     * elements. scc_rbmap_insert is the closest thing */
    scc_rbmap_insert(&map, ups, 0);

    /* Omitting 3) */

    print_map("4) Updated map:", map);

    /* scc_rbmap_remove supplies the functionality of
     * std::map::erase */
    scc_rbmap_remove(map, gpu);
    print_map("5) After erase:", map);

    /* std::erase_if is best emulated by manual iteration. The
     * rbmap must not be modified while iterating over it, As such,
     * an scc_vec is used to track the keys to be removed */
    scc_vec(char const *) vec = scc_vec_new(char const *);
    scc_rbmap_iter(char const *, int) mapiter;

    /* Find and record the keys to be removed */
    scc_rbmap_foreach(mapiter, map) {
        if(mapiter->value > 25) {
            scc_vec_push(&vec, mapiter->key);
        }
    }

    /* And remove them */
    char const **veciter;
    scc_vec_foreach(veciter, vec) {
        scc_rbmap_remove(map, *veciter);
    }

    /* Don't need the keys anymore */
    scc_vec_free(vec);

    print_map("6) After erase:", map);

    printf("7) scc_rbmap_size(map) = %zu\n", scc_rbmap_size(map));

    scc_rbmap_clear(map);
    printf("8) Map is empty: %s\n", scc_rbmap_empty(map) ? "true" : "false");

    /* Free the map */
    scc_rbmap_free(map);
}

/* ============= OUTPUT =============== */
// STDOUT:1) Initial map: [CPU] = 10; [GPU] = 15; [RAM] = 20;
// STDOUT:2) Updated map: [CPU] = 25; [GPU] = 15; [RAM] = 20; [SSD] = 30;
// STDOUT:4) Updated map: [CPU] = 25; [GPU] = 15; [RAM] = 20; [SSD] = 30; [UPS] = 0;
// STDOUT:5) After erase: [CPU] = 25; [RAM] = 20; [SSD] = 30; [UPS] = 0;
// STDOUT:6) After erase: [CPU] = 25; [RAM] = 20; [UPS] = 0;
// STDOUT:7) scc_rbmap_size(map) = 3
// STDOUT:8) Map is empty: true
/* ==================================== */

// RUN: %cc -I %root %s %libscc_a -g -o %litbuild/cppreference_map_ex
// RUN: %litbuild/cppreference_map_ex | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STDOUT
