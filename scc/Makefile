ifdef __node

__scc_incdir := $(__node_path)
__public_headers := $(addsuffix .h,$(addprefix $(__node_path)/scc_,hashmap hashtab rbtree stack svec vec))

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
