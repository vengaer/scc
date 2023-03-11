ifndef __Deps_mk
__Deps_mk := _

btmap_deps           := algorithm arena svec
btree_deps           := algorithm arena svec vec
hashmap_deps         := arch canary
hashtab_deps         := arch canary
rbmap_deps           := arena rbtree ringdeque
rbtree_deps          := arena ringdeque
stack_deps           := svec
hashmap_simdemu_deps := canary hashmap swvec
hashtab_simdemu_deps := canary hashtab swvec

endif # __Deps_mk
