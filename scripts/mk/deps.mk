ifndef __Deps_mk
__Deps_mk := _

avx2_deps            := arch canary hash hashmap hashtab swar
btmap_deps           := algorithm arena vec
btree_deps           := algorithm arena vec
hashmap_deps         := arch canary hash hashtab swar
hashtab_deps         := arch canary hash hashmap swar
rbmap_deps           := arena deque rbtree
rbtree_deps          := arena deque
stack_deps           := vec
hashmap_swar_deps    := arch canary hash hashmap hashtab swar
hashtab_swar_deps    := arch canary hash hashmap hashtab swar

endif # __Deps_mk
