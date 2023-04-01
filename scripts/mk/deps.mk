ifndef __Deps_mk
__Deps_mk := _

avx2_deps            := arch canary hash hashmap hashtab swvec
btmap_deps           := algorithm arena vec
btree_deps           := algorithm arena vec
hashmap_deps         := arch canary hash hashtab swvec
hashtab_deps         := arch canary hash hashmap swvec
rbmap_deps           := arena rbtree ringdeque
rbtree_deps          := arena ringdeque
stack_deps           := vec
hashmap_simdemu_deps := arch canary hash hashmap hashtab swvec
hashtab_simdemu_deps := arch canary hash hashmap hashtab swvec

endif # __Deps_mk
