docbuilddir := $(builddir)/docs
dirs        += $(docbuilddir)
docindex    := $(docbuilddir)/index.$(htmlext)

.PHONY: all
all:

.PHONY: doc
doc: $(patsubst $(docdir)/%.$(adocext),$(docbuilddir)/%.$(htmlext),$(wildcard $(docdir)/*.$(adocext)))

.PHONY: docshow
docshow: $(docindex) doc
	$(if $(BROWSER),,$(error BROWSER environment variable not set))
	$(BROWSER) $<

$(docbuilddir)/%.$(htmlext): $(docdir)/%.$(adocext) | $(docbuilddir)
	$(info [ADOC] $(notdir $@))
	$(ADOC) -o $@ $^
