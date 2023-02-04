ifdef __node

$(call push,hashtab_deps)

$(if $(call not,$(__simd_enabled)),$(eval hashtab_deps += swvec))

$(call decl-unit)
$(call decl-mutate)

$(call pop,hashtab_deps)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
