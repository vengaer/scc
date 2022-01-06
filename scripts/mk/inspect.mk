inspectbuilddir := $(builddir)/inspect
inspectobj      := $(patsubst $(inspectdir)/%.$(cext),$(inspectbuilddir)/%.$(oext),$(wildcard $(inspectdir)/*.$(cext)))
dirs            += $(inspectbuilddir)

libinspect      := $(inspectbuilddir)/libsccinspect.a

.PHONY: all
all:

.PHONY: lint
lint: CPPFLAGS += -I$(testdir)

$(libinspect): $(inspectobj)
	$(info [AR] $(notdir $@))
	$(AR) $(ARFLAGS) -o $@ $^

$(inspectbuilddir)/%.$(oext): $(inspectdir)/%.$(cext) | $(inspectbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^
