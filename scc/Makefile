ifdef __node

__config_header  := $(__node_path)/scc_config.h

__scc_incdir     := $(__node_path)
__public_headers := $(addsuffix .h,$(addprefix $(__node_path)/scc_,btmap btree hashmap hashtab rbmap rbtree ringdeque stack svec vec))

$(__config_header): $(pyscripts)/confgen.$(pyext) $(archinfo) $(simdinfo) $(vecinfo) $(CONFIG)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $< $(sover) $(filter-out $<,$^) -o $@
	touch $@

.PHONY: __clean_config_header
__clean_config_header:
	$(RM) $(RMFLAGS) $(__config_header)

clean: __clean_config_header

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
