#ifndef BTMAP_COMPAT_H
#define BTMAP_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

void *btmap_new(void);
void btmap_free(void *btmap);

#ifdef __cplusplus
}
#endif

#endif /* BTMAP_COMPAT_H */
