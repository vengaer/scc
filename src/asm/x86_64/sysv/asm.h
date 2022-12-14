#ifndef ASM_H
#define ASM_H

#define i64 long long
#define u64 unsigned long long

i64 scc_hashtab_probe_insert(void const *base, void const *handle, u64 elemsize, u64 hash);
i64 scc_hashtab_probe_find(void const *base, void const *handle, u64 elemsize, u64 hash);
u64 scc_hashmap_probe_insert(void const *base, void const *map, u64 keysize, u64 hash);
i64 scc_hashmap_probe_find(void const *base, void const *map, u64 keysize, u64 hash);

#undef u64
#undef i64

#endif /* ASM_H */
