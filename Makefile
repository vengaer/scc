-include .config.mk

CC           := clang
AS           := $(CC)
LD           := $(CC)
AR           := ar
RB           := ruby
CMAKE        := cmake
ADOC         := asciidoc
TIDY         := clang-tidy

MKDIR        := mkdir
LN           := ln
RM           := rm
TOUCH        := touch
PYTHON       := python
PYTEST       := pytest
PYLINT       := pylint

root         := $(abspath $(CURDIR))
builddir     := $(root)/build
sccbuilddir  := $(builddir)/scc
dirs         += $(builddir) $(sccbuilddir)

scc          := scc
srcdir       := $(root)/src
headerdir    := $(root)/scc
testdir      := $(root)/test
unitdir      := $(testdir)/unit
fuzzdir      := $(testdir)/fuzz
inspectdir   := $(testdir)/inspect
panictestdir := $(testdir)/panic
doctestdir   := $(testdir)/docs
docdir       := $(root)/docs

scripts      := $(root)/scripts
mkscripts    := $(scripts)/mk
pyscripts    := $(scripts)/python

mach         := $(root)/mach
asmach       := $(mach)/asm

cext         := c
hext         := h
oext         := o
asext        := S
aext         := a
dext         := d
mkext        := mk
soext        := so
sover        := 0
socompat     := 0
adocext      := adoc
htmlext      := html
jsonext      := json
pyext        := py

solink       := $(builddir)/lib$(scc).$(soext)
solib        := $(solink).$(sover)
alib         := $(builddir)/lib$(scc).$(aext)

ASFLAGS      := -c -g
CFLAGS       := -std=c99 -c -MD -MP -g -Wall -Wextra -Wpedantic -Waggregate-return   \
                -Wbad-function-cast -Wcast-qual -Wfloat-equal -Wmissing-include-dirs \
                -Wnested-externs -Wpointer-arith -Wshadow -Wunknown-pragmas -Wswitch \
                -Wundef -Wunused -Wwrite-strings
CPPFLAGS     := -I$(root) -DNDEBUG
LDFLAGS      :=
LDLIBS       :=
ARFLAGS      := -rcs

so_CFLAGS    := -fPIC
so_LDFLAGS   := -shared -Wl,-soname,lib$(scc).$(soext).$(socompat)

TIDYFLAGS     = --warnings-as-errors=* --checks=$(tidychecks)

MKDIRFLAGS   := -p
LNFLAGS      := -sf
RMFLAGS      := -rf
TOUCHFLAGS   :=
PYTESTFLAGS  := -v
PYLINTFLAGS  := --fail-under=10.0

sccobj       := $(patsubst $(srcdir)/%.$(cext),$(sccbuilddir)/%.$(oext),$(wildcard $(srcdir)/*.$(cext)))
obj          += $(sccobj)

.SECONDEXPANSION:
include $(mkscripts)/deps.$(mkext)
include $(mkscripts)/inspect.$(mkext)
include $(mkscripts)/mach.$(mkext)
include $(mkscripts)/fuzz.$(mkext)
include $(mkscripts)/unit.$(mkext)
include $(mkscripts)/docs.$(mkext)
include $(mkscripts)/lint.$(mkext)
include $(mkscripts)/panic.$(mkext)

.PHONY: all
all: $(alib) $(solink)

$(solink): $(solib)
	$(info [LN] $(notdir $@))
	$(LN) $(LNFLAGS) $< $@

$(solib): $(sccobj)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(so_LDFLAGS) $(LDLIBS)

$(alib): $(sccobj)
	$(info [AR] $(notdir $@))
	$(AR) $(ARFLAGS) -o $@ $^

$(sccbuilddir)/%.$(oext): $(srcdir)/%.$(cext) | $(sccbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(so_CFLAGS) $(CPPFLAGS) $< -o $@

$(dirs):
	$(MKDIR) $(MKDIRFLAGS) $@

.PHONY: check
check: CPPFLAGS := $(filter-out -DNDEBUG,$(CPPFLAGS))

.PHONY: fuzz
fuzz:  CPPFLAGS := $(filter-out -DNDEBUG,$(CPPFLAGS))

.PHONY: lint
lint:

.PHONY: clean
clean:
	$(RM) $(RMFLAGS) $(builddir) $(solink) $(solib) $(alib)

.PHONY: distclean
distclean: clean

$(VERBOSE).SILENT:

$(obj): Makefile $(wildcard $(mkscripts)/*.$(mkext))
-include $(patsubst %.$(oext),%.$(dext),$(obj))
