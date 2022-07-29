ifdef __node

$(call push,btree_deps)
btree_deps += vec

$(call decl-fuzz)

$(call pop,btree_deps)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
