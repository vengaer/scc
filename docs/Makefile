ifdef __node

SPHINX   := sphinx-build

__conf       := $(__node_path)/conf.$(pyext)
__docdir     := $(__node_path)
__docgen     := $(patsubst %.$(hext),$(__node_builddir)/%.$(rstext),$(notdir $(__public_headers)))
__docs       := $(call wildcard-artifacts,$(__node_path),$(rstext),$(htmlext)) \
                $(patsubst %.$(rstext),%.$(htmlext),$(__docgen))
__sccdoc     := $(pyscripts)/sccdoc.py
__node_src   := $(wildcard $(__node_path)/*.$(rstext)) $(__docgen)

.SECONDARY: $(__docgen)

$(call include-node,hashmap)
$(call include-node,hashtab)
$(call include-node,rbtree)
$(call include-node,stack)
$(call include-node,svec)
$(call include-node,vec)

$(__docs)&: $(__node_src) $(__all_rstsrc) $(__docgen) $(__conf) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,docs)
	$(SPHINX) -b html $(__docdir) $(dir $@)
	$(TOUCH) $(__docs)

$(__node_builddir)/%.$(rstext): $(__scc_incdir)/%.$(hext) $(__sccdoc) $(CONFIG) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(__sccdoc) $< -o $@ $(if $(__devel),-d)

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
