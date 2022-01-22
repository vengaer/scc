ifdef __node

inspectdir := $(__node_path)/inspect

$(call include-node,buildsystem)
#$(call include-node,docs)
#$(call include-node,fuzz)
#$(call include-node,inspect)
#$(call include-node,panic)
#$(call include-node,perf)
$(call include-node,unit)

.PHONY: check
check: $(call reeval,__node_path,check)
check: CPPFLAGS += -I$(__node_path)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
