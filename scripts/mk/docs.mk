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

.PHONY: docshow
docshow: $(docindex) doc
	$(if $(BROWSER),,$(error BROWSER environment variable not set))
	$(BROWSER) $<

$(docbuilddir)/%.$(htmlext): $(docdir)/%.$(adocext) | $(docbuilddir)
	$(info [ADOC] $(notdir $@))
	$(ADOC) -o $@ $^

define snip-linker-rules
$(strip
    $(foreach __t,$(snips),
        $(eval
            $(__t): $(__t).$(cext) $(alib)
	            $$(info [CC] $$(notdir $$@))
	            $$(CC) $$(CPPFLAGS) -o $$@ $$^

            $(__t).$(cext): $(snipdir)/$(notdir $(__t)).$(snipext) $(snipgen) | $(snipbuilddir)
	            $$(info [PY] $$(notdir $$@))
	            $$(snipgen) -o $$@ $$<

        check: $(__t))))
endef

$(call snip-linker-rules)
