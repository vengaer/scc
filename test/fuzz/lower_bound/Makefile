ifdef __node

$(call push,lower_bound_deps)
lower_bound_deps += algorithm

$(call decl-fuzz)

$(call pop,lower_bound_deps)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
