ifndef __Deps_mk
__Deps_mk := _

avx2_deps            := arch canary hash hashmap hashtab murmur32 murmur64 swar
bloom_deps           := arch canary hash murmur32 murmur64 swar
btmap_deps           := algorithm arena vec
btree_deps           := algorithm arena vec
hashmap_deps         := arch canary hash hashtab murmur32 murmur64 swar
hashtab_deps         := arch canary hash hashmap murmur32 murmur64 swar
rbmap_deps           := arena deque rbtree
rbtree_deps          := arena deque
stack_deps           := vec
hashmap_swar_deps    := arch canary hash hashmap hashtab murmur32 murmur64 swar
hashtab_swar_deps    := arch canary hash hashmap hashtab murmur32 murmur64 swar

endif # __Deps_mk
