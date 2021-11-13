CC         := gcc
LD         := $(CC)
AR         := ar

MKDIR      := mkdir
LN         := ln
RM         := rm

root       := $(abspath $(CURDIR))
builddir   := $(root)/build

scc        := scc
srcdir     := $(root)/src
sccdir     := $(root)/$(scc)

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
              -Wundef -Wunused -Wwrite-strings -fPIC
CPPFLAGS   := -I$(sccdir)
LDFLAGS    := -shared -Wl,-soname,lib$(scc).$(soext).$(socompat)
LDLIBS     :=
ARFLAGS    := -rcs


MKDIRFLAGS := -p
LNFLAGS    := -sf
RMFLAGS    := -rf

obj        := $(patsubst $(srcdir)/%.$(cext),$(builddir)/%.$(oext),$(wildcard $(srcdir)/*.$(cext)))

.PHONY: all
all: $(alib) $(solink)

$(solink): $(solib)
	$(info [LN] $(notdir $@))
	$(LN) $(LNFLAGS) $< $@

$(solib): $(obj)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(alib): $(obj)
	$(info [AR] $(notdir $@))
	$(AR) $(ARFLAGS) -o $@ $^

$(builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(builddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@

$(builddir):
	$(MKDIR) $(MKDIRFLAGS) $@

.PHONY: clean
clean:
	$(RM) $(RMFLAGS) $(builddir) $(solink) $(solib) $(alib)

$(VERBOSE).SILENT:
