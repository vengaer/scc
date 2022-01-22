ifdef __node

runnersuffix    := _runner

CFLAGS          += -fsanitize=address,undefined
LDFLAGS         += -fsanitize=address,undefined -L$(unitydir)
LDLIBS          += -lunity

$(call include-node,arena)
$(call include-node,mem)
$(call include-node,rbtree)
$(call include-node,stack)
$(call include-node,svec)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory -s
endif
