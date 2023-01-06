ifndef __Deps_mk
__Deps_mk := _

btmap_deps           := algorithm arena svec
btree_deps           := algorithm arena svec vec
rbmap_deps           := arena rbtree
rbtree_deps          := arena
stack_deps           := svec
hashmap_simdemu_deps := hashmap
hashtab_simdemu_deps := hashtab

endif # __Deps_mk
