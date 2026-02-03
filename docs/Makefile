ifdef __node

doxydir      := $(builddir)/docs/doxygen
doxyxml      := $(doxydir)/xml
dirs         += $(doxyxml)

docdir       := $(__node_path)
doxyfile     := $(docdir)/Doxyfile

DOXYGEN      ?= doxygen
SPHINX_BUILD ?= sphinx-build

rstsrc       := $(wildcard $(__node_path)/*.rst)

$(call include-node,reference)

$(doxyxml)/index.$(xmlext): $(doxyfile) $(__public_headers) | $(doxyxml)
	$(call echo-gen,$(notdir $@))
	$(DOXYGEN) -q $(doxyfile)

$(__node_builddir)/index.html: $(rstsrc) $(doxyxml)/index.$(xmlext)
	$(call echo-gen,$(notdir $@))
	$(SPHINX_BUILD) -b html $(docdir) $(dir $@)

docs: $(__node_builddir)/index.html

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
