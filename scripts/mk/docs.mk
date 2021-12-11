refdir         := $(docdir)/reference
designdir      := $(docdir)/design

docbuilddir    := $(builddir)/docs
refbuilddir    := $(docbuilddir)/reference
designbuilddir := $(docbuilddir)/design
snipbuilddir   := $(refbuilddir)/snips
dirs           += $(docbuilddir) $(snipbuilddir) $(refbuilddir) $(designbuilddir)
docsrcdirs     := $(docdir) $(refdir) $(designdir)

docindex       := $(docbuilddir)/index.$(htmlext)
snipgen        := $(pyscripts)/snipgen.py

snipext        := csnip
snipdir        := $(refdir)/snips
symmap         := $(snipdir)/symmap.$(jsonext)
funcdefs       := $(snipdir)/funcs.$(jsonext)
docsnips       := $(wildcard $(snipdir)/*.$(snipext))
snips          := $(patsubst $(snipdir)/%.$(snipext),$(snipbuilddir)/%,$(docsnips))
htmlpgs        := $(patsubst $(docdir)/%.$(adocext),$(docbuilddir)/%.$(htmlext),$(wildcard $(addsuffix /*.$(adocext),$(docsrcdirs))))

.PHONY: all
all:

.PHONY: doc
doc: $(htmlpgs)

.PHONY: show
show: $(docindex) doc
	$(if $(BROWSER),,$(error BROWSER environment variable not set))
	$(BROWSER) $<

.PHONY: __chk_docs
check:
	$(PYTEST) $(PYTESTFLAGS) --builddir=$(snipbuilddir) $(doctestdir)

check: __chk_docs

$(refbuilddir)/%.$(htmlext): $(refdir)/%.$(adocext) $(docsnips) | $(refbuilddir)
	$(info [ADOC] $(notdir $@))
	$(ADOC) -o $@ $<

$(designbuilddir)/%.$(htmlext): $(designdir)/%.$(adocext) | $(designbuilddir)
	$(info [ADOC] $(notdir $@))
	$(ADOC) -o $@ $<

$(docbuilddir)/%.$(htmlext): $(docdir)/%.$(adocext) | $(refbuilddir)
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
	            $$(PYTHON) $$(snipgen) -o $$@ -s $$(symmap) -f $$(funcdefs) $$<

            __chk_docs: $(__t))))
endef

$(call snip-linker-rules)
