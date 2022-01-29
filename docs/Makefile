ifdef __node

docindex := $(__node_builddir)/index.$(htmlext)
docdir   := $(__node_path)

$(call include-node,design)
$(call include-node,reference)

$(__node_builddir)/%.$(htmlext): $(__node_path)/%.$(adocext) $(__all_mkfiles) | $(__node_builddir)
	$(info [ADOC] $(notdir $@))
	$(ADOC) -o $@ $< $(ADOCFLAGS)

.PHONY: doc
doc: $(docindex)

.PHONY: show
show: $(call require,show,BROWSER)
show: $(docindex)
	$(BROWSER) $<

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
