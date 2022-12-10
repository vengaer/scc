ifdef __node

$(call push,btmap_deps)
btmap_deps += vec

$(call decl-unit)

$(call pop,btmap_deps)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
