ifdef __node

SPHINX := sphinx-build

__docs := $(call wildcard-artifacts,$(__node_path),$(rstext),$(htmlext))
__conf := $(__node_path)/conf.$(pyext)

$(__docs)&: $(wildcard $(__node_path)/*.$(rstext)) $(__conf) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(SPHINX) -b html $(dir $<) $(dir $@)
	$(TOUCH) $(__docs)

docs: $(__docs)

docshow: $(call require,docshow,BROWSER)
docshow: $(__node_builddir)/index.$(htmlext) docs
	$(BROWSER) $<

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
