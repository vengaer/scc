#ifndef ASM_H
#define ASM_H

#define i64 long long
#define u64 unsigned long long

i64 scc_hashtab_probe_insert(void *base, void *handle, u64 elemsize, u64 hash);

#undef u64
#undef i64

#endif /* ASM_H */
