ifdef __node

$(call assert,$(abi_lower))

ifneq ($(wildcard $(__node_path)/$(abi_lower)),)
  $(call include-node,$(abi_lower))
endif

else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
