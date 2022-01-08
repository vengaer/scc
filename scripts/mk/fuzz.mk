LLVM_COV           := llvm-cov
LLVM_PROFDATA      := llvm-profdata

CORPUSDIR          := $(fuzzdir)/corpora/$(SCC_FUZZ)

FUZZTIME           := 360
FUZZLEN            := 32768
FUZZTIMEOUT        := 10
FUZZERFLAGS        := -max_len=$(FUZZLEN) -max_total_time=$(FUZZTIME) -use_value_profile=1 \
                      -timeout=$(FUZZTIMEOUT) $(CORPUSDIR)

sanitizers         := address undefined fuzzer

fuzzsrcdir         := $(fuzzdir)/$(SCC_FUZZ)
fuzzbuilddir       := $(builddir)/fuzz/$(SCC_FUZZ)
dirs               += $(fuzzbuilddir)

__comma            :=,
fsanitize          := -fsanitize=$(subst $(subst ,, ),$(__comma),$(addsuffix ,$(sanitizers)))
fuzzflags          := $(fsanitize) -fprofile-instr-generate -fcoverage-mapping
mergeflags         := -merge=1 $(CORPUSDIR) $(SCC_CORPORA)

fuzztarget         := $(fuzzbuilddir)/scc_fuzz_$(SCC_FUZZ)
fuzzobj            := $(patsubst $(fuzzsrcdir)/%.$(cext),$(fuzzbuilddir)/%.$(oext),$(wildcard $(fuzzsrcdir)/*.$(cext)))

__srcdirs          := $(srcdir) $(assrcdir) $(fuzzsrcdir) $(inspectdir)

proffile           := LLVM_PROFILE_FILE
export $(proffile) := $(builddir)/.fuzz.profraw
profdata           := $(builddir)/.fuzz.profdata

LLVM_PROFFLAGS     := merge -sparse $($(proffile)) -o $(profdata)
LLVM_COVFLAGS      := $(fuzztarget) -instr-profile=$(profdata)
LLVM_COV_SHOW      := show $(LLVM_COVFLAGS)
LLVM_COV_REPORT    := report $(LLVM_COVFLAGS)

.PHONY: all
all:

.PHONY: fuzz
fuzz: $$(call require-fuzztarget,fuzz)
fuzz: CC         := clang
fuzz: LD         := clang
fuzz: CFLAGS     += $(fuzzflags)
fuzz: CPPFLAGS   += -I$(testdir)
fuzz: LDFLAGS    += $(fuzzflags)
fuzz: fuzztarget := $(fuzzbuilddir)/$(SCC_FUZZ)
fuzz: $(fuzztarget)
	$^ $(FUZZERFLAGS)
	$(LLVM_PROFDATA) $(LLVM_PROFFLAGS)
	$(LLVM_COV) $(LLVM_COV_SHOW)
	$(LLVM_COV) $(LLVM_COV_REPORT)

.PHONY: merge
merge: $$(call require-fuzztarget,merge)
merge: $$(call require-corpora,merge)
merge: CC         := clang
merge: LD         := clang
merge: CFLAGS     += $(fuzzflags)
merge: CPPFLAGS   += -I$(testdir)
merge: LDFLAGS    += $(fuzzflags)
merge: fuzztarget := $(fuzzbuilddir)/$(SCC_FUZZ)
merge: $(fuzztarget)
	$^ $(mergeflags)

$(fuzzbuilddir)/%.$(oext): $(fuzzsrcdir)/%.$(cext) | $(fuzzbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

$(fuzztarget): $(fuzzobj)
	$(info [LD] $(notdir $(fuzzbuilddir))/$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

define require-fuzztarget
$(if $(findstring _-$(1)_-,_-$(MAKECMDGOALS)_-),$(if $(SCC_FUZZ),,$(error SCC_FUZZ is empty)))
endef

define require-corpora
$(if $(findstring _-$(1)_-,_-$(MAKECMDGOALS)_-),$(if $(SCC_CORPORA),,$(error SCC_CORPORA is empty)))
endef

define fuzz-dependency-rules
$(strip
    $(eval $(SCC_FUZZ)_deps += $(SCC_FUZZ))
    $(foreach __dep,$($(SCC_FUZZ)_deps),
        $(foreach __dir,$(__srcdirs),
            $(eval __src := $(__dir)/scc_$(__dep).$(cext))
            $(if $(wildcard $(__src)),
                $(eval
                    $(eval __o := $(patsubst $(__dir)/%.$(cext),$(fuzzbuilddir)/%.$(oext),$(__src)))
                    $(eval obj += $(__o))
                    $(__o): $(__src) | $(fuzzbuilddir)
	                    $$(info [CC] $(notdir $(fuzzbuilddir))/$$(notdir $$@))
	                    $$(CC) $$(CFLAGS) $$(CPPFLAGS) -o $$@ $$<

                    $(fuzztarget): $(__o)))

            $(eval __src := $(__dir)/scc_$(__dep).$(asext))
            $(if $(wildcard $(__src)),
                $(eval
                    $(eval __o := $(patsubst $(__dir)/%.$(asext),$(fuzzbuilddir)/%.$(oext),$(__src)))
                    $(eval obj += $(__o))
                    $(__o): $(__src) | $(fuzzbuilddir)
	                    $$(info [AS] $(notdir $(fuzzbuilddir))/$$(notdir $$@))
	                    $$(AS) $$(ASFLAGS) -o $$@ $$<

                    $(fuzztarget): $(__o))))))
endef

$(call fuzz-dependency-rules)
