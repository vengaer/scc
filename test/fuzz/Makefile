ifdef __node

LLVM_COV           := llvm-cov
LLVM_PROFDATA      := llvm-profdata

CORPUSDIR          := $(__node_path)/corpora/$(SCC_FUZZ_TARGET)

SCC_FUZZTIME       ?= 360
SCC_FUZZLEN        ?= 32768
SCC_FUZZTIMEO      ?= 10
FUZZERFLAGS        := -max_len=$(SCC_FUZZLEN) -max_total_time=$(SCC_FUZZTIME) -use_value_profile=1 \
                      -timeout=$(SCC_FUZZTIMEO) $(CORPUSDIR)

fuzzflags          := -fsanitize=fuzzer,undefined,address -fprofile-instr-generate -fcoverage-mapping
mergeflags         := -merge=1 $(CORPUSDIR) $(SCC_FUZZ_CORPORA)

dirs               += $(CORPUSDIR)

proffile           := LLVM_PROFILE_FILE
$(proffile)         = $(__node_builddir)/fuzz.profraw
profdata            = $(__node_builddir)/fuzz.profdata

LLVM_PROFFLAGS      = merge -sparse $($(proffile)) -o $(profdata)
LLVM_COVFLAGS       = $(fuzztarget) -instr-profile=$(profdata)
LLVM_COV_SHOW       = $(LLVM_COV) show $(LLVM_COVFLAGS)
LLVM_COV_REPORT     = $(LLVM_COV) report $(LLVM_COVFLAGS)

define __fuzz-obj
$(strip
    $(patsubst $(srcdir)/%.$(cext),$(__node_builddir)/%.$(oext),
        $(wildcard $(patsubst %,$(srcdir)/%*.$(cext),$(__node) $($(__node)_deps))))
    $(patsubst $(srcdir)/%.$(cext),$(__node_builddir)/%.$(oext),
        $(wildcard $(addsuffix .$(cext),$(addprefix $(srcdir)/*,$($(__node)_deps)))))
    $(patsubst $(asdir)/%.$(asext),$(__node_builddir)/%.$(oext),
        $(wildcard $(patsubst %,$(asdir)/%*.$(asext),$(__node) $($(__node)_deps) $(simd_isa)_isa)))
    $(patsubst $(inspectdir)/%.$(cext),$(__node_builddir)/%.$(oext),
        $(wildcard $(inspectdir)/*$(__node)*.$(cext)))
    $(if $($(__node)_deps),
        $(patsubst $(inspectdir)/%.$(cext),$(inspectbuilddir)/%.$(oext),
            $(wildcard $(addprefix $(inspectdir)/*,$(addsuffix *.$(cext),$($(__node)_deps))))))
    $(call wildcard-obj,$(__node_path),$(cext)))
endef

define decl-fuzz
$(eval
    $(eval fuzztarget := $(__node_builddir)/fuzz_$(__node))

    $(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(inspectdir)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(simdemudir)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(asdir)/%.$(asext) | $(__node_builddir)
	    $$(call echo-as,$(__node)/$$(notdir $$@))
	    $$(AS) -o $$@ $$< $(ASFLAGS) $(CPPFLAGS)

    $(eval __fuzzobj := $(call __fuzz-obj))
    $(eval __all_obj += $(__fuzzobj))

    $(fuzztarget): $(__fuzzobj)
	    $$(call echo-ld,$(__node)/$$(notdir $$@))
	    $$(CC) -o $$@ $$^ $(LDFLAGS) $(LDLIBS)

    fuzz: $(fuzztarget) | $(CORPUSDIR)
	    $(proffile)=$($(proffile)) $$< $(FUZZERFLAGS)
	    $(LLVM_PROFDATA) $(LLVM_PROFFLAGS)
	    $(LLVM_COV_SHOW)
	    $(LLVM_COV_REPORT)

    merge: $(fuzztarget)
	    $(proffile)=$($(proffile)) $$< $(mergeflags))
endef

fuzz: $$(call require,fuzz,SCC_FUZZ_TARGET)

merge: $$(call require,merge,SCC_FUZZ_TARGET)
merge: $$(call require,merge,SCC_FUZZ_CORPORA)

$(call push,CC)
$(call push,LD)
$(call push,CFLAGS)
$(call push,CPPFLAGS)
$(call push,LDFLAGS)

CC                 := clang
LD                 := clang
CFLAGS             += $(fuzzflags) -DSCC_INTERCEPT_NULLSIZE_COPIES
CPPFLAGS           := -I$(testdir) $(filter-out -DNDEBUG,$(CPPFLAGS)) -I$(__node_path)
LDFLAGS            += $(fuzzflags)

$(call include-node,$(SCC_FUZZ_TARGET))

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
