ifdef __node

$(call push,stack_deps)
stack_deps += vec

$(call decl-unit)
$(call decl-mutate)

$(call pop,stack_deps)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
