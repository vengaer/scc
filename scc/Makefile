ifdef __node

config_header        := $(__node_path)/config.$(hext)
__config_cache       := $(builddir)/.scconfig.cache
__scconfig           := $(pyscripts)/scconfig.py
__config_opts        := -g SCC_CONFIG_H -c $(__config_cache) -l $(builddir)/.scconfig.lock

__config_header_init := $(builddir)/.config.init.stamp

__scc_incdir         := $(__node_path)
__public_headers     := $(addsuffix .h,btmap btree hashmap hashtab rbmap rbtree ringdeque stack svec vec)

$(__config_header_init): $(wildcard $(__config_module)/*.$(pyext))
	$(call echo-gen,$(notdir $(config_header)))
	$(PYTHON) $(__scconfig) $(__config_opts) -o $(config_header) init
	$(TOUCH) $@

$(config_header): $(__config_header_init) $(config_header_entries)
	$(PYTHON) $(__scconfig) $(__config_opts) commit

__clean_config_header:
	$(RM) $(RMFLAGS) $(config_header)
	$(RM) $(RMFLAGS) $(__config_cache)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
