#ifndef HASHTAB_INSERTION_H
#define HASHTAB_INSERTION_H

#include "bmtypes.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

void *hashtab_insertion_init();
void hashtab_insertion_free(void *table);
bool hashtab_insertion_benchmark(void **table, bm_type const* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* HASHTAB_INSERTION_H */
