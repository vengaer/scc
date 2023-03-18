#ifndef HASHTAB_COMPAT_H
#define HASHTAB_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

void *hashtab_new(void);
void hashtab_free(void *hashtab);

#ifdef __cplusplus
}
#endif

#endif /* HASHTAB_COMPAT_H */
