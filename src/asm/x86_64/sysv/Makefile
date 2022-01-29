ifdef __node

__node_obj := $(call wildcard-obj,$(__node_path),$(asext))

$(solib) $(alib): $(__node_obj)

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(asext) | $(__node_builddir)
	$(info [AS]   $(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
