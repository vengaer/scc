#ifndef BTREE_COMPAT_H
#define BTREE_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

void *btree_new(void);
void btree_free(void *btree);

#ifdef __cplusplus
}
#endif

#endif /* BTREE_COMPAT_H */
