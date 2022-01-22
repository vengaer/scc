ifdef __node

__chk_stack   := $(__node_builddir)/.chk.stamp
__node_obj    := $(call wildcard_obj,$(__node_path),$(cext))                 \
                 $(call wildcard_obj,$(__node_path),$(cext),$(runnersuffix)) \
                 $(call objpath,$(srcdir),scc_svec scc_vec,$(cext))
stacktest     := $(__node_builddir)/stacktest

__stackprefix := $(notdir $(__node_builddir))

$(stacktest): $(call reeval,LDLIBS,$(stacktest))
$(stacktest): $(call reeval,LDFLAGS,$(stacktest))
$(stacktest): $(__node_obj) | $(__node_builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(call reeval,CFLAGS,$(__node_builddir)/%.$(oext))

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__stackprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%$(runnersuffix).$(cext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [RB] $(__stackprefix)/$(notdir $@) (unit))
	$(RB) $(unitygenerator) $< $@

$(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__stackprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(__node_builddir)/%.$(cext)
	$(info [CC] $(__stackprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(inspectdir)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__stackprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__chk_stack): $(stacktest)
	$<
	$(TOUCH) $@

.PHONY: check
check: $(__chk_stack)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory -s
endif
