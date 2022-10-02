ifdef __node

__config_header  := $(__node_path)/scc_config.h

__scc_incdir     := $(__node_path)
__public_headers := $(addsuffix .h,$(addprefix $(__node_path)/scc_,btree hashmap hashtab rbtree ringdeque stack svec vec))

$(__config_header): $(pyscripts)/confgen.$(pyext) $(archinfo) $(simdinfo) $(vecinfo)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $< $(filter-out $<,$^) -o $@
	touch $@

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
