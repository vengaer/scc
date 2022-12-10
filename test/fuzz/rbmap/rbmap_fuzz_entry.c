#include <fuzzer/assertion.h>
#include <fuzzer/dbg.h>
#include <inspect/rbtree_inspect.h>
#include <scc/rbmap.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static size_t partition(uint32_t *data, size_t size);
static int compare(void const *l, void const *r);
static void fuzz_insertion(void *map, uint32_t const *data, size_t ue, size_t size);
static void fuzz_traversal(void *map, uint32_t *data, size_t ue);
static void fuzz_removal(void *map, uint32_t const *data, size_t ue);

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) {
    uint32_t *buf = 0;
    size = size / sizeof(*buf);
    if(!size) {
        return 0;
    }

    buf = malloc(size * sizeof(*buf));
    if(!buf) {
        return 0;
    }

    memcpy(buf, data, size * sizeof(*buf));
    unsigned unique_end = partition(buf, size);

    uint32_t const *it = 0;
    dbg_pr("Data:\n");
    dbg_pr_n(it, buf, size, "%" PRIu32 " ", *it);
    dbg_pr("\n");

    scc_rbmap(uint32_t, uint32_t) rbmap = scc_rbmap_new(uint32_t, uint32_t, compare);

    fuzz_insertion(rbmap, buf, unique_end, size);
    fuzz_traversal(rbmap, buf, unique_end);
    fuzz_removal(rbmap, buf, unique_end);

    free(buf);
    scc_rbmap_free(rbmap);
    return 0;
}

static size_t partition(uint32_t *data, size_t size) {
    bool unique = true;
    unsigned unique_end = 0u;
    /* The most stupid partitioning scheme ever */
    for(unsigned i = 0u; i < size; ++i) {
        unique = true;
        for(unsigned j = 0u; j < i; ++j) {
            if(data[i] == data[j]) {
                unique = false;
                break;
            }
        }
        if(unique) {
            data[unique_end++] = data[i];
        }
    }
    return unique_end;
}

static int compare(void const *l, void const *r) {
    uint32_t leftval = *(uint32_t const *)l;
    uint32_t rightval = *(uint32_t const *)r;
    return (leftval < rightval ? -1 : 1) * !(leftval == rightval);
}

static void fuzz_insertion(void *map, uint32_t const *data, size_t ue, size_t size) {
    scc_rbmap(uint32_t, uint32_t) rbmap = map;

    scc_inspect_mask mask;
    uint32_t *val;
    for(unsigned i = 0u; i < ue; ++i) {
        fuzz_assert(
            scc_rbmap_insert(&rbmap, data[i], data[i] << 1u),
            "Could not insert (%" PRIu32 ", %" PRIu32 ") in the map",
            data[i], data[i] << 1u
        );

        fuzz_assert(
            scc_rbmap_size(rbmap) == i + 1u,
            "Invalid size %zu after %u insertions", scc_rbmap_size(rbmap), i + 1u
        );

        mask = scc_rbtree_inspect_properties(rbmap);
        fuzz_assert(
            !mask,
            "Properties violated upon insertion of %" PRIu32 ", mask %#" PRIx32,
            data[i], (uint32_t)mask
        );

        for(unsigned j = 0u; j < i; ++j) {
            val = scc_rbmap_find(rbmap, data[j]);
            fuzz_assert(val, "Key %" PRIu32 " not found in map", data[j]);
            fuzz_assert(
                *val == data[j] << 1u,
                "Key %" PRIu32 " maps to unexpected value %" PRIu32, data[j], *val
            );
        }
    }

    size_t mapsize = scc_rbmap_size(rbmap);
    for(unsigned i = ue; i < size; ++i) {
        fuzz_assert(
            scc_rbmap_insert(&rbmap, data[i], 0u),
            "Could not overwrite value for key %" PRIu32, data[i]
        );

        fuzz_assert(
            scc_rbmap_size(rbmap) == mapsize,
            "Map size increased on overwrite"
        );

        for(unsigned j = ue; j < i; ++j) {
            val = scc_rbmap_find(rbmap, data[j]);
            fuzz_assert(val, "Key %" PRIu32 " not found in map", data[j]);
            fuzz_assert(!*val, "Value mapping to key %" PRIu32 " not cleared", data[j]);
        }
    }

    for(unsigned i = 0u; i < ue; ++i) {
        fuzz_assert(scc_rbmap_insert(&rbmap, data[i], data[i] << 1u));
    }
}

static void fuzz_traversal(void *map, uint32_t *data, size_t ue) {
    scc_rbmap(uint32_t, uint32_t) rbmap = map;

    qsort(data, ue, sizeof(*data), compare);

    scc_rbmap_iter(uint32_t, uint32_t) iter;
    scc_rbmap_foreach(iter, rbmap) {
        fuzz_assert(iter->key == *data, "foreach key mismatch");
        fuzz_assert(iter->value == *data << 1u, "foreach value mismatch");
        ++data;
    }

    scc_rbmap_foreach_reversed(iter, rbmap) {
        --data;
        fuzz_assert(iter->key == *data, "foreach_rev key mismatch");
        fuzz_assert(iter->value == *data << 1u, "foreach_rev value mismatch");
    }
}

static void fuzz_removal(void *map, uint32_t const *data, size_t ue) {
    scc_rbmap(uint32_t, uint32_t) rbmap = map;
    for(unsigned i = 0u; i < ue; ++i) {
        fuzz_assert(scc_rbmap_size(rbmap) == ue - i, "Size mismatch");
        fuzz_assert(
            scc_rbmap_remove(rbmap, data[i]),
            "Error removing %" PRIu32, data[i]
        );
    }

    fuzz_assert(!scc_rbmap_size(rbmap), "Map not empty");
}
