#include <scc/algorithm.h>
#include <fuzzer/assertion.h>
#include <fuzzer/dbg.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int compare(void const *left, void const *right) {
    int64_t diff = *(uint16_t const *)left - *(uint16_t const *)right;
    if(diff < 0) {
        return -1;
    }
    else if(diff > 0) {
        return 1;
    }
    return 0;
}

/* Shitty sorting */
static unsigned unique_sort(uint16_t *data, size_t size) {
    bool unique = true;
    unsigned unique_end = 0u;
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

    qsort(data, unique_end, sizeof(data[0]), compare);
    return unique_end;
}

static size_t shitty_lower_bound(uint16_t val, uint16_t const *data, size_t size) {
    for(size_t i = 0u; i < size; ++i) {
        if(data[i] >= val) {
            return i;
        }
    }
    return size;
}

int LLVMFuzzerTestOneInput(uint8_t const *data, size_t size) {
    size /= sizeof(uint16_t);
    if(!size) {
        return 0;
    }
    uint16_t *u16s = malloc(size * sizeof(*u16s));
    if(!u16s) {
        fputs("key malloc failure\n", stderr);
        abort();
    }

    memcpy(u16s, data, size * sizeof(*u16s));

    size = unique_sort(u16s, size);

    uint16_t *kit;
    dbg_pr("Values:\n");
    dbg_pr_n(kit, u16s, size, "%" PRIu16 " ", *u16s);
    dbg_pr("\n");

    uint16_t end = u16s[size - 1u];
    if(end < UINT16_MAX - 1u) {
        end += 2u;
    }

    size_t shitty;
    size_t bins;
    for(uint16_t i = 0u; i < end; ++i) {
        shitty = shitty_lower_bound(i, u16s, size);
        bins = scc_algo_lower_bound(&i, u16s, size, sizeof(i), compare);
        fuzz_assert(shitty == bins, "Shitty: %zu Binary search: %zu", shitty, bins);
    }

    free(u16s);
    return 0;
}
