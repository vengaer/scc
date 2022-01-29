ifdef __node

$(call push,rbtree_deps)
rbtree_deps += svec

$(call decl-fuzz)

$(call pop,rbtree_deps)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
