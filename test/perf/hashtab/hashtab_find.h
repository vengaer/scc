#ifndef HASHTAB_FIND_H
#define HASHTAB_FIND_H

#include "bmtypes.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

void *hashtab_find_init();
void hashtab_find_free(void *tab);
void hashtab_find_insert_all(void **tab, bm_type const *data, size_t size);
void hashtab_find_clear(void *tab);
bool hashtab_find_benchmark(void *tab, bm_type const *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* HASHTAB_FIND_H */
