ifdef __node

SPHINX   := sphinx-build

$(call include-node,hashtab)

__conf   := $(__node_path)/conf.$(pyext)
__docdir := $(__node_path)
__docs   := $(patsubst %.$(rstext),$(__node_builddir)/$(notdir %).$(htmlext),$(__all_rstsrc))

$(__docs)&: $(__all_rstsrc) $(__conf) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(SPHINX) -b html $(__docdir) $(dir $@)
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
