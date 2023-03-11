ifdef __node

__swvec_entry         := $(__node_builddir)/.swvec.stamp
config_header_entries += $(__swvec_entry)

simdemudir            := $(__node_path)
simdemubuilddir       := $(__node_builddir)

__node_obj := $(call wildcard-obj,$(__node_path),$(cext))

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(call echo-cc,$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__swvec_entry): $(__config_header_init) | $(__node_builddir)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_SWVEC_SIZE "sizeof(unsigned long long)" -C "Size of vectors used when emulating SIMD in software"
	$(TOUCH) $@

$(solib) $(alib): $(__node_obj)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
