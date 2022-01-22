ifdef __node

__chk_mem    := $(__node_builddir)/.chk.stamp
__node_obj   := $(call wildcard_obj,$(__node_path),$(cext))                 \
                $(call wildcard_obj,$(__node_path),$(cext),$(runnersuffix))
memtest      := $(__node_builddir)/memtest

__memprefix := $(notdir $(__node_builddir))

$(memtest): $(call reeval,LDLIBS,$(memtest))
$(memtest): $(call reeval,LDFLAGS,$(memtest))
$(memtest): $(__node_obj) | $(__node_builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(call reeval,CFLAGS,$(__node_builddir)/%.$(oext))

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [CC] $(__memprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__node_builddir)/%$(runnersuffix).$(cext): $(__node_path)/%.$(cext) | $(__node_builddir)
	$(info [RB] $(__memprefix)/$(notdir $@) (unit))
	$(RB) $(unitygenerator) $< $@
	$(TOUCH) $@

$(__node_builddir)/%.$(oext): $(__node_builddir)/%.$(cext)
	$(info [CC] $(__memprefix)/$(notdir $@) (unit))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

$(__chk_mem): $(memtest)
	$<
	$(TOUCH) $@

.PHONY: check
check: $(__chk_mem)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory -s
endif
