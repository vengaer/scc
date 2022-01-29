ifdef __node

runnersuffix    := _runner

$(call push,CFLAGS)
$(call push,LDFLAGS)
$(call push,LDLIBS)

CFLAGS          += -fsanitize=address,undefined
LDFLAGS         += -fsanitize=address,undefined -L$(unitydir)
LDLIBS          += -lunity

$(call include-node,arena)
$(call include-node,mem)
$(call include-node,rbtree)
$(call include-node,stack)
$(call include-node,svec)
$(call include-node,vec)

$(call pop,LDLIBS)
$(call pop,LDFLAGS)
$(call pop,CFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
