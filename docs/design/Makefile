ifdef __node

$(__node_builddir)/%.$(htmlext): $(__node_path)/%.$(adocext) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(ADOC) $(ADOCFLAGS) -o $@ $<

$(docindex): $(call wildcard-artifacts,$(__node_path),$(adocext),$(htmlext))

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
