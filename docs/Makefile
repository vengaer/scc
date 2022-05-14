ifdef __node

SPHINX   := sphinx-build

__conf       := $(__node_path)/conf.$(pyext)
__docdir     := $(__node_path)
__docgen     := $(patsubst %.$(hext),$(__node_builddir)/%_$(hext).$(rstext),$(notdir $(__public_headers))) \
                $(patsubst %.$(cext),$(__node_builddir)/%_$(cext).$(rstext),$(notdir $(wildcard $(patsubst %.$(hext),%.$(cext),$(addprefix $(__scc_srcdir)/,$(notdir $(__public_headers)))))))
__docs       := $(call wildcard-artifacts,$(__node_path),$(rstext),$(htmlext)) \
                $(patsubst %.$(rstext),%.$(htmlext),$(__docgen))
sccdoc       := $(pyscripts)/sccdoc.$(pyext)
confdoc      := $(pyscripts)/confdoc.$(pyext)
__node_src   := $(wildcard $(__node_path)/*.$(rstext)) $(__docgen)
__all_rstsrc += $(__node_builddir)/conftool_spec.$(rstext)

.SECONDARY: $(__docgen)

$(call include-node,configuration)
$(call include-node,hashmap)
$(call include-node,hashtab)
$(call include-node,rbtree)
$(call include-node,stack)
$(call include-node,svec)
$(call include-node,vec)

$(__docs)&: $(__node_src) $(__all_rstsrc) $(__docgen) $(__conf) | $(__node_builddir)
	$(call echo-gen,docs)
	$(SPHINX) -b html $(__docdir) $(dir $@)
	$(TOUCH) $(__docs)

$(__node_builddir)/%_$(hext).$(rstext): $(__scc_incdir)/%.$(hext) $(sccdoc) $(CONFIG) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $(sccdoc) $< -o $@ $(if $(__devel_docs),-d)

$(__node_builddir)/%_$(cext).$(rstext): $(__scc_srcdir)/%.$(cext) $(sccdoc) $(CONFIG) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $(sccdoc) $< -o $@ $(if $(__devel_docs),-d)

$(__node_builddir)/conftool_spec.$(rstext): $(__conftool_spec) $(confdoc) $(__all_mkfiles) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $(confdoc) -s $< -o $@

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
