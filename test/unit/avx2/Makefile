ifdef __node

$(call push,avx2_deps)

avx2_deps += canary hashtab

$(call decl-unit)

$(call pop,avx2_deps)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
