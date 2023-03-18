ifdef __node

testdir    := $(__node_path)
inspectdir := $(__node_path)/inspect

$(call include-node,inspect)

$(call include-node,assert)
$(call include-node,buildsystem)
$(call include-node,fuzz)
$(call include-node,unit)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
