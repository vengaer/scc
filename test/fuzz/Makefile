ifdef __node

LLVM_COV           := llvm-cov
LLVM_PROFDATA      := llvm-profdata

CORPUSDIR          := $(__node_path)/corpora/$(SCC_FUZZ)

FUZZTIME           := 360
FUZZLEN            := 32768
FUZZTIMEOUT        := 10
FUZZERFLAGS        := -max_len=$(FUZZLEN) -max_total_time=$(FUZZTIME) -use_value_profile=1 \
                      -timeout=$(FUZZTIMEOUT) $(CORPUSDIR)

fuzzflags          := -fsanitize=fuzzer,undefined,address -fprofile-instr-generate -fcoverage-mapping
mergeflags         := -merge=1 $(CORPUSDIR) $(SCC_CORPORA)

proffile           := LLVM_PROFILE_FILE
export $(proffile) := $(builddir)/.fuzz.profraw
profdata           := $(builddir)/.fuzz.profdata

LLVM_PROFFLAGS     := merge -sparse $($(proffile)) -o $(profdata)
LLVM_COVFLAGS       = $(fuzztarget) -instr-profile=$(profdata)
LLVM_COV_SHOW       = show $(LLVM_COVFLAGS)
LLVM_COV_REPORT     = report $(LLVM_COVFLAGS)

define __fuzz-obj
$(strip
    $(patsubst $(srcdir)/%.$(cext),$(__node_builddir)/%.$(oext),
        $(wildcard $(srcdir)/scc_$(__node).$(cext)))
    $(patsubst $(srcdir)/%.$(cext),$(__node_builddir)/%.$(oext),
        $(wildcard $(addsuffix .$(cext),$(addprefix $(srcdir)/scc_,$($(__node)_deps)))))
    $(patsubst $(asdir)/%.$(asext),$(__node_builddir)/%.$(oext),
        $(wildcard $(asdir)/scc_$(__node)*.$(asext)))
    $(patsubst $(inspectdir)/%.$(cext),$(__node_builddir)/%.$(oext),
        $(wildcard $(inspectdir)/scc_$(__node)*.$(cext)))
    $(call wildcard_obj,$(__node_path),$(cext)))
endef

define decl-fuzz
$(eval
    $(eval fuzztarget := $(__node_builddir)/fuzz_$(__node))

    $(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	    $$(info [CC] $(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	    $$(info [CC] $(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(inspectdir)/%.$(cext) | $(__node_builddir)
	    $$(info [CC] $(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(asdir)/%.$(asext) | $(__node_builddir)
	    $$(info [AS] $(__node)/$$(notdir $$@))
	    $$(AS) -o $$@ $$< $(ASFLAGS)

    $(fuzztarget): $(call __fuzz-obj)
	    $$(info [LD] $(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$^ $(LDFLAGS) $(LDLIBS)

    fuzz: $(fuzztarget)
	    $$< $(FUZZERFLAGS)
	    $(LLVM_PROFDATA) $(LLVM_PROFFLAGS)
	    $(LLVM_COV) $(LLVM_COV_SHOW)

    merge: $(fuzztarget)
	    $$< $(mergeflags))
endef

fuzz: $$(call require,fuzz,SCC_FUZZ)

merge: $$(call require,merge,SCC_FUZZ)
merge: $$(call require,merge,SCC_CORPORA)

$(call push,CC)
$(call push,LD)
$(call push,CFLAGS)
$(call push,CPPFLAGS)
$(call push,LDFLAGS)

CC                 := clang
LD                 := clang
CFLAGS             += $(fuzzflags)
CPPFLAGS           := -I$(testdir) $(filter-out -DNDEBUG,$(CPPFLAGS))
LDFLAGS            += $(fuzzflags)

$(call include-node,$(SCC_FUZZ))

$(call pop,LDFLAGS)
$(call pop,CPPFLAGS)
$(call pop,CFLAGS)
$(call pop,LD)
$(call pop,CC)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
