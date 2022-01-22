ifdef __node

__chk_svec   := $(__node_builddir)/.chk.stamp
__node_obj   := $(call wildcard_obj,$(__node_path),$(cext))                 \
                $(call wildcard_obj,$(__node_path),$(cext),$(runnersuffix)) \
                $(call objpath,$(srcdir),scc_svec,$(cext))
svectest     := $(__node_builddir)/svectest

__svecprefix := $(notdir $(__node_builddir))

$(svectest): $(call reeval,LDLIBS,$(svectest))
$(svectest): $(call reeval,LDFLAGS,$(svectest))
$(svectest): $(__node_obj) | $(__node_builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(call reeval,CFLAGS,$(__node_builddir)/%.$(oext))

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__svecprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%$(runnersuffix).$(cext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [RB] $(__svecprefix)/$(notdir $@) (unit))
	$(RB) $(unitygenerator) $< $@

$(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__svecprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(__node_builddir)/%.$(cext)
	$(info [CC] $(__svecprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(inspectdir)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__svecprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__chk_svec): $(svectest)
	$<
	$(TOUCH) $@

.PHONY: check
check: $(__chk_svec)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory -s
endif
