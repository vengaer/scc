.SECONDEXPANSION:

VER_MAJOR    := 0
VER_MINOR    := 0
VER_PATCH    := 1

CC           := clang
CXX          := clang++
AS           := $(CC)
LD           := $(CC)
AR           := ar
RB           := ruby
CMAKE        := cmake
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
dirs         += $(builddir)
srcdir       := $(root)/src

TIDYFLAGS     = --warnings-as-errors=* --checks=$(tidychecks)

MKDIRFLAGS   := -p
LNFLAGS      := -sf
RMFLAGS      := -rf
TOUCHFLAGS   :=
PYTESTFLAGS  := -v
PYLINTFLAGS  := --fail-under=10.0

submodules   := submodules/.init.stamp

scc          := scc

scripts      := $(root)/scripts
mkscripts    := $(scripts)/mk
pyscripts    := $(scripts)/py

.PHONY: all
all:

include $(wildcard $(mkscripts)/*.mk)

ASFLAGS      := -c -x assembler -MD -MP $(__debug_info)
CFLAGS       := -std=c99 -c -MD -MP -Wall -Wextra -Wpedantic -Waggregate-return      \
                -Wbad-function-cast -Wcast-qual -Wfloat-equal -Wmissing-include-dirs \
                -Wnested-externs -Wpointer-arith -Wshadow -Wunknown-pragmas -Wswitch \
                -Wundef -Wunused -Wwrite-strings $(__debug_info)
CXXFLAGS     := -std=c++17 -c -MD -MP -Wall -Wextra -Wpedantic $(__debug_info)
CPPFLAGS     += -I$(root) -DNDEBUG

LDFLAGS      :=
LDLIBS       :=
ARFLAGS      := -rcs

sover        := $(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)
socompat     := $(VER_MAJOR)

solink       := $(builddir)/lib$(scc).$(soext)
solinkcompat := $(solink).$(VER_MAJOR)
solib        := $(solink).$(sover)
alib         := $(builddir)/lib$(scc).$(aext)

.PHONY: all
all: $(alib) $(solink)

$(call include-node,arch)
$(call include-node,scc,inc)
$(call include-node,src,scc)
$(call include-node,submodules)
$(call include-node,test)
$(call include-node,docs)

$(dirs):
	$(MKDIR) $(MKDIRFLAGS) $@

.PHONY: benchmark
benchmark: CFLAGS   += -O3
benchmark: CXXFLAGS += -O3

.PHONY: check
check: CPPFLAGS := $(filter-out -DNDEBUG,$(CPPFLAGS))

.PHONY: fuzz
fuzz:  CPPFLAGS := $(filter-out -DNDEBUG,$(CPPFLAGS))

.PHONY: mutate
mutate: CPPFLAGS := $(filter-out -DNDEBUG,$(CPPFLAGS))

.PHONY: lint
lint:

.PHONY: clean
clean:
	$(RM) $(RMFLAGS) $(builddir)

.PHONY: distclean
distclean: clean

.PHONY: docs
docs:

tags: FORCE
	$(call echo-gen,$@)
	$(CTAGS) $(CTAGSFLAGS)

FORCE:

.PHONY: docker-image
docker-image:

.PHONY: docker
docker:

$(VERBOSE).SILENT:

$(__all_obj): $(__config_header)
$(__all_obj): $(__all_mkfiles) $(wildcard $(mkscripts)/*.$(mkext)) $(wildcard $(root)/.config)
$(__all_obj): $(__validate_config)
-include $(patsubst %.$(oext),%.$(dext),$(__all_obj))
