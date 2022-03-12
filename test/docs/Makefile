ifdef __node

__chk_docs := $(__node_builddir)/.chk.stamp
doctestdir := $(__node_path)

snipgen    := $(pyscripts)/snipgen.$(pyext)
symmap     := $(snipdir)/symmap.$(jsonext)
funcdefs   := $(snipdir)/funcs.$(jsonext)

snipobj    := $(call wildcard-obj,$(snipdir),$(snipext))
snipbins   := $(subst .$(oext),,$(snipobj))

scc_stack_bad_container_deps := vec

define __snip-deps
$$(patsubst $(srcdir)/%.$(cext),$(sccbuilddir)/%.$(oext),
    $$(wildcard $(srcdir)/scc_$$(call secondword,$$(subst _, ,$$(notdir $(1)))).$(cext))
    $$(wildcard $(srcdir)/scc_$$($$(call secondword,$$(subst _, ,$$(notdir $(1))))_deps).$(cext))
    $$(wildcard $(srcdir)/scc_$$($$(notdir $(1))_deps).$(cext)))
endef

$(__chk_docs): $(snipbins) $(wildcard $(__node_path)/*.$(pyext))
	$(PYTEST) $(PYTESTFLAGS) --builddir=$(dir $@) $(doctestdir)
	$(TOUCH) $@

$(snipbins): $$@.$(oext) $(call __snip-deps,$$@)
	$(call echo-ld,$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(snipobj): $$(patsubst %.$(oext),%.$(cext),$$@)
	$(call echo-cc,$(notdir $@))
	$(CC) -o $@ $< -c $(CPPFLAGS)

$(__node_builddir)/%.$(cext): $(snipdir)/%.$(snipext) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(snipgen) -o $@ -s $(symmap) -f $(funcdefs) $<

# Don't remove temporary files
.SECONDARY: $(call wildcard-artifacts,$(snipdir),$(snipext),$(cext))

.PHONY: check
check: $(__chk_docs)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
