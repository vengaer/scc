.SECONDEXPANSION:

include .version

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
PYTHON       := python3
PYTEST       := pytest
PYLINT       := pylint

root         := $(abspath $(CURDIR))
builddir     := $(root)/build
dirs         += $(builddir)
srcdir       := $(root)/lib

scripts      := $(root)/scripts
mkscripts    := $(scripts)/mk
pyscripts    := $(scripts)/py

TIDYFLAGS     = --warnings-as-errors=* --checks=$(tidychecks)

MKDIRFLAGS   := -p
LNFLAGS      := -sf
RMFLAGS      := -rf
TOUCHFLAGS   :=
PYTESTFLAGS  := -v
PYLINTFLAGS  := --fail-under=10.0 --rcfile $(pyscripts)/.pylintrc --score=n

submodules   := submodules/.init.stamp

scc          := scc

.PHONY: all
all:

include $(wildcard $(mkscripts)/*.mk)

ASFLAGS      := -c -MD -MP $(__debug_info)
CFLAGS       := -std=c99 -c -MD -MP -Wall -Wextra -Wpedantic -Waggregate-return
CFLAGS       += -Wbad-function-cast -Wcast-qual -Wfloat-equal -Wmissing-include-dirs
CFLAGS       += -Wnested-externs -Wpointer-arith -Wshadow -Wunknown-pragmas -Wswitch
CFLAGS       += -Wundef -Wunused -Wwrite-strings -fPIC
CFLAGS       += $(__werror) $(__debug_info)
CXXFLAGS     := -std=c++17 -c -MD -MP -Wall -Wextra -Wpedantic $(__debug_info)
CPPFLAGS     += -I$(root) -DNDEBUG

LDFLAGS      := -fPIC
LDLIBS       +=
ARFLAGS      := -rcs

sover        := $(SCC_MAJOR).$(SCC_MINOR).$(SCC_PATCH)
socompat     := $(SCC_MAJOR)

solink       := $(builddir)/lib$(scc).$(soext)
solinkcompat := $(solink).$(SCC_MAJOR)
solib        := $(solink).$(sover)
alib         := $(builddir)/lib$(scc).$(aext)

.PHONY: all
all: $(alib) $(solink)

$(call include-node,arch)
$(call include-node,scc)
$(call include-node,submodules)
$(call include-node,lib)
$(call include-node,test)
$(call include-node,docs)
$(call include-node,benchmark)

$(dirs):
	$(MKDIR) $(MKDIRFLAGS) $@

.PHONY: benchmark
benchmark: CFLAGS   += -O3
benchmark: CXXFLAGS += -O3

.PHONY: check
check: CPPFLAGS := $(filter-out -DNDEBUG,$(CPPFLAGS))
check: $(alib) $(solib)
	lit -vv --config-prefix=.lit $(root) -o $(builddir)/lit.json

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

$(__all_obj): $(config_header)
$(__all_obj): $(__all_mkfiles) $(wildcard $(mkscripts)/*.$(mkext))
$(__all_obj): $(config_stamp)
-include $(patsubst %.$(oext),%.$(dext),$(__all_obj))
