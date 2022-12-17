ifdef __node

asdir      := $(__node_path)/$(arch_lower)/$(abi_lower)
asbuilddir := $(__node_builddir)/$(arch_lower)/$(abi_lower)

$(call include-node-if,$(__simd_enable),$(arch_lower))

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
