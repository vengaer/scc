ifdef __node

__swar_entry          := $(__node_builddir)/.swar.stamp
config_header_entries += $(__swar_entry)

$(call push,CFLAGS)
$(call push,LDFLAGS)

CFLAGS       += -fPIC
LDFLAGS      += -shared -Wl,-soname,lib$(scc).$(soext).$(socompat) -Wl,--no-undefined

__node_obj   := $(call wildcard-obj,$(__node_path),$(cext))
__scc_srcdir := $(__node_path)
sccbuilddir  := $(__node_builddir)

$(solink): $(solinkcompat) | $(builddir)
	$(call echo-ln,$(notdir $@))
	$(LN) $(LNFLAGS) $< $@

$(solinkcompat): $(solib) | $(builddir)
	$(call echo-ln,$(notdir $@))
	$(LN) $(LNFLAGS) $< $@

$(solib): $(call reeval,LDFLAGS,$(solib))
$(solib): $(__node_obj) | $(builddir)
	$(call echo-ld,$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(alib): $(__node_obj) | $(builddir)
	$(call echo-ar,$(notdir $@))
	$(AR) $(ARFLAGS) -o $@ $^

$(__node_builddir)/%.$(oext): $(call reeval,CFLAGS,$(__node_builddir)/%.$(oext))
$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(call echo-cc,$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__swar_entry): $(__config_header_init) | $(__node_builddir)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_SWARVEC_SIZE "sizeof(unsigned long long)" -C "Size of SWAR vectors"
	$(TOUCH) $@

$(call pop,LDFLAGS)
$(call pop,CFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
