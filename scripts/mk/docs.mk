docbuilddir  := $(builddir)/docs
snipbuilddir := $(docbuilddir)/snips
dirs         += $(docbuilddir) $(snipbuilddir)

docindex     := $(docbuilddir)/index.$(htmlext)
snipgen      := $(pyscripts)/snipgen.py

snipext      := csnip
snipdir      := $(docdir)/snips
docsnips     := $(wildcard $(snipdir)/*.$(snipext))
snips        := $(patsubst $(snipdir)/%.$(snipext),$(snipbuilddir)/%,$(docsnips))
htmlpgs      := $(patsubst $(docdir)%.$(adocext),$(docbuilddir)/%.$(htmlext),$(wildcard $(docdir)/*.$(adocext)))

.PHONY: all
all:

.PHONY: doc
doc: $(htmlpgs)

.PHONY: show
show: $(docindex) doc
	$(if $(BROWSER),,$(error BROWSER environment variable not set))
	$(BROWSER) $<

.PHONY: __chk_docs
__chk_docs:
	$(PYTEST) $(PYTESTFLAGS) --builddir=$(snipbuilddir) $(doctestdir)

check: __chk_docs

$(docbuilddir)/%.$(htmlext): $(docdir)/%.$(adocext) $(docsnips) | $(docbuilddir)
	$(info [ADOC] $(notdir $@))
	$(ADOC) -o $@ $<

define snip-linker-rules
$(strip
    $(foreach __t,$(snips),
        $(eval
            $(__t): $(__t).$(cext) $(alib)
	            $$(info [CC] $$(notdir $$@))
	            $$(CC) $$(CPPFLAGS) -g -o $$@ $$^

            $(__t).$(cext): $(snipdir)/$(notdir $(__t)).$(snipext) $(snipgen) | $(snipbuilddir)
	            $$(info [PY] $$(notdir $$@))
	            $$(PYTHON) $$(snipgen) -o $$@ $$<

            __chk_docs: $(__t)

            )))
endef

$(call snip-linker-rules)
