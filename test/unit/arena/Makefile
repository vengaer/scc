ifdef __node

__chk_arena   := $(__node_builddir)/.chk.stamp
__node_obj    := $(call wildcard_obj,$(__node_path),$(cext))                 \
                 $(call wildcard_obj,$(__node_path),$(cext),$(runnersuffix)) \
                 $(call objpath,$(srcdir),scc_arena,$(cext))
arenatest     := $(__node_builddir)/arenatest

__arenaprefix := $(notdir $(__node_builddir))

$(arenatest): $(call reeval,LDLIBS,$(arenatest))
$(arenatest): $(call reeval,LDFLAGS,$(arenatest))
$(arenatest): $(__node_obj) | $(__node_builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(call reeval,CFLAGS,$(__node_builddir)/%.$(oext))

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__arenaprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__arenaprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%$(runnersuffix).$(cext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [RB] $(__arenaprefix)/$(notdir $@) (unit))
	$(RB) $(unitygenerator) $< $@

$(__node_builddir)/%.$(oext): $(__node_builddir)/%.$(cext)
	$(info [CC] $(__arenaprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__chk_arena): $(arenatest)
	$<
	$(TOUCH) $@

.PHONY: check
check: $(__chk_arena)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
