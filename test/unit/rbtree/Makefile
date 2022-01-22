ifdef __node

__chk_rbtree   := $(__node_builddir)/.chk.stamp
__node_obj     := $(call wildcard_obj,$(__node_path),$(cext))                     \
                  $(call wildcard_obj,$(__node_path),$(cext),$(runnersuffix))     \
                  $(call objpath,$(srcdir),scc_rbtree scc_svec scc_arena,$(cext)) \
                  $(call objpath,$(inspectdir),scc_rbtree_inspect,$(cext))
rbtreetest     := $(__node_builddir)/rbtreetest

__rbtreeprefix := $(notdir $(__node_builddir))

$(rbtreetest): $(call reeval,LDLIBS,$(rbtreetest))
$(rbtreetest): $(call reeval,LDFLAGS,$(rbtreetest))
$(rbtreetest): $(__node_obj) | $(__node_builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(call reeval,CFLAGS,$(__node_builddir)/%.$(oext))

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__rbtreeprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%$(runnersuffix).$(cext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [RB] $(__rbtreeprefix)/$(notdir $@) (unit))
	$(RB) $(unitygenerator) $< $@

$(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__rbtreeprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(__node_builddir)/%.$(cext)
	$(info [CC] $(__rbtreeprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%.$(oext): $(inspectdir)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__rbtreeprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__chk_rbtree): $(rbtreetest)
	$<
	$(TOUCH) $@

.PHONY: check
check: $(__chk_rbtree)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory -s
endif
