ifdef __node

config_header        := $(__node_path)/config.$(hext)
__config_cache       := $(builddir)/.scconfig.cache
__scconfig           := $(pyscripts)/scconfig.py
__config_opts        := -g SCC_CONFIG_H -c $(__config_cache) -l $(builddir)/.scconfig.lock

__config_header_init := $(__node_builddir)/.config.init.stamp
__config_header_ver  := $(__node_builddir)/.config.ver.stamp

__scc_incdir         := $(__node_path)
__public_headers     := $(addsuffix .h,btmap btree hashmap hashtab rbmap rbtree ringdeque stack svec vec)

$(__config_header_init): $(wildcard $(__config_module)/*.$(pyext)) | $(__node_builddir)
	$(call echo-gen,$(notdir $(config_header)))
	$(PYTHON) $(__scconfig) $(__config_opts) -o $(config_header) init
	$(TOUCH) $@

$(__config_header_ver): $(__config_header_init)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_VERSION_MAJOR $(VER_MAJOR)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_VERSION_MINOR $(VER_MINOR)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_VERSION_PATCH $(VER_PATCH)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_VERSION "(((SCC_VERSION_MAJOR & 0xff) << 24) | ((SCC_VERSION_MINOR & 0xff) << 16) | SCC_VERSION_PATCH & 0xffff)"
	$(TOUCH) $@

$(config_header): $(__config_header_init) $(__config_header_ver) $(config_header_entries)
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
