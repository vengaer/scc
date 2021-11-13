CC         := gcc
LD         := $(CC)
AR         := ar
RB         := ruby
CMAKE      := cmake

MKDIR      := mkdir
LN         := ln
RM         := rm
TOUCH      := touch

root       := $(abspath $(CURDIR))
builddir   := $(root)/build
dirs       += $(builddir)

scc        := scc
srcdir     := $(root)/src
testdir    := $(root)/test
unitdir    := $(testdir)/unit

cext       := c
oext       := o
aext       := a
soext      := so
sover      := 0
socompat   := 0

solink     := lib$(scc).$(soext)
solib      := $(solink).$(sover)
alib       := lib$(scc).$(aext)

CFLAGS     := -std=c99 -c -MD -MP -g -Wall -Wextra -Wpedantic -Waggregate-return   \
              -Wbad-function-cast -Wcast-qual -Wfloat-equal -Wmissing-include-dirs \
              -Wnested-externs -Wpointer-arith -Wshadow -Wunknown-pragmas -Wswitch \
              -Wundef -Wunused -Wwrite-strings
CPPFLAGS   := -I$(root)
LDFLAGS    :=
LDLIBS     :=
ARFLAGS    := -rcs

so_CFLAGS  := -fPIC
so_LDFLAGS := -shared -Wl,-soname,lib$(scc).$(soext).$(socompat)

MKDIRFLAGS := -p
LNFLAGS    := -sf
RMFLAGS    := -rf
TOUCHFLAGS :=

obj        := $(patsubst $(srcdir)/%.$(cext),$(builddir)/%.$(oext),$(wildcard $(srcdir)/*.$(cext)))

include scripts/unit.mk

.PHONY: all
all: $(alib) $(solink)

$(solink): $(solib)
	$(info [LN] $(notdir $@))
	$(LN) $(LNFLAGS) $< $@

$(solib): $(obj)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(so_LDFLAGS) $(LDLIBS)

$(alib): $(obj)
	$(info [AR] $(notdir $@))
	$(AR) $(ARFLAGS) -o $@ $^

$(builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(builddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(so_CFLAGS) $(CPPFLAGS) $^ -o $@

$(dirs):
	$(MKDIR) $(MKDIRFLAGS) $@

.PHONY: check
check:

.PHONY: clean
clean:
	$(RM) $(RMFLAGS) $(builddir) $(solink) $(solib) $(alib)

.PHONY: distclean
distclean: clean

$(VERBOSE).SILENT:
