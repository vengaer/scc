ifdef __node

config_header           := $(__node_path)/config.$(hext)
__config_cache          := $(builddir)/.scconfig.cache
__scconfig              := $(pyscripts)/scconfig.py
__config_opts           := -g SCC_CONFIG_H -c $(__config_cache) -l $(builddir)/.scconfig.lock

__config_header_ver     := $(__node_builddir)/.config.ver.stamp
__config_header_bitarch := $(__node_builddir)/.config.bitarch.stamp
__config_header_libm    := $(__node_builddir)/.config.libm.stamp
__config_header_types   := $(__node_builddir)/.config.types.stamp

__scc_incdir            := $(__node_path)
__public_headers        := $(addprefix $(__node_path)/,$(addsuffix .h,bloom btmap btree hashmap hashtab rbmap rbtree deque stack vec))

$(__config_header_init): $(wildcard $(__config_module)/*.$(pyext)) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,$(notdir $(config_header)))
	$(PYTHON) $(__scconfig) $(__config_opts) -o $(config_header) init
	$(TOUCH) $@

$(__config_header_ver): $(__config_header_init)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_VERSION_MAJOR $(SCC_MAJOR) -C "Library major version"
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_VERSION_MINOR $(SCC_MINOR) -C "Library minor version"
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_VERSION_PATCH $(SCC_PATCH) -C "Library patch version"
	$(TOUCH) $@

$(__config_header_libm): $(__config_header_init)
	$(if $(__have_libm),$(PYTHON) $(__scconfig) $(__config_opts) add SCC_HAVE_LIBM -C "Linked against libm")
	$(TOUCH) $@

$(__config_header_bitarch): $(__config_header_init)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_BITARCH_$(bitarch) -C "System architecture"
	$(TOUCH) $@

$(__config_header_types): $(__config_header_init)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_$(if $(u32_available),HAVE,NO)_UINT32_T -C "Have uint32_t?"
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_$(if $(u64_available),HAVE,NO)_UINT64_T -C "Have uint64_t?"
	$(TOUCH) $@

$(config_header): $(__config_header_init) $(__config_header_ver) $(config_header_entries) $(__config_header_bitarch) $(__config_header_libm) $(__config_header_types)
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
