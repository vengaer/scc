LLVM_COV           := llvm-cov
LLVM_PROFDATA      := llvm-profdata

FUZZTIME           := 480
FUZZLEN            := 32768
FUZZTIMEOUT        := 10
FUZZFLAGS          := -max_len=$(FUZZLEN) -max_total_time=$(FUZZTIME) -use_value_profile=1 \
                      -timeout=$(FUZZTIMEOUT) $(CORPUSDIR)

sanitizers         := address undefined fuzzer

__comma            :=,
fsanitize          := -fsanitize=$(subst $(subst ,, ),$(__comma),$(addsuffix ,$(sanitizers)))
fuzzflags          := $(fsanitize) -fprofile-instr-generate -fcoverage-mapping

fuzzsrcdir         := $(fuzzdir)/$(SCC_FUZZ)
fuzzbuilddir       := $(builddir)/fuzz/$(SCC_FUZZ)
dirs               += $(fuzzbuilddir)

fuzztarget         := $(fuzzbuilddir)/scc_fuzz_$(SCC_FUZZ)
fuzzobj            := $(patsubst $(fuzzsrcdir)/%.$(cext),$(fuzzbuilddir)/%.$(oext),$(wildcard $(fuzzsrcdir)/*.$(cext)))

srcdirs            := $(srcdir) $(fuzzsrcdir) $(inspectdir)

proffile           := LLVM_PROFILE_FILE
export $(proffile) := $(builddir)/.fuzz.profraw
profdata           := $(builddir)/.fuzz.profdata

LLVM_PROFFLAGS     := merge -sparse $(LLVM_PROFILE_FILE) -o $(profdata)
LLVM_COVFLAGS      := $(fuzztarget) -instr-profile=$(profdata)
LLVM_COV_SHOW      := show $(LLVM_COVFLAGS)
LLVM_COV_REPORT    := report $(LLVM_COVFLAGS)

.PHONY: all
all:

.PHONY: fuzz
fuzz: $$(if $$(findstring _-$$@_-,_-$$(MAKECMDGOALS)_-),\
          $$(if $(SCC_FUZZ),,$$(error SCC_FUZZ is empty)))
fuzz: CC         := clang
fuzz: LD         := clang
fuzz: CFLAGS     += $(fuzzflags)
fuzz: CPPFLAGS   += -I$(testdir)
fuzz: LDFLAGS    += $(fuzzflags)
fuzz: fuzztarget := $(fuzzbuilddir)/$(SCC_FUZZ)
fuzz: $(fuzztarget)
	$^ $(FUZZFLAGS)
	$(LLVM_PROFDATA) $(LLVM_PROFFLAGS)
	$(LLVM_COV) $(LLVM_COV_SHOW)
	$(LLVM_COV) $(LLVM_COV_REPORT)

$(fuzzbuilddir)/%.$(oext): $(fuzzsrcdir)/%.$(cext) | $(fuzzbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

$(fuzztarget): $(fuzzobj)
	$(info [LD] $(notdir $(fuzzbuilddir))/$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

define fuzz-dependency-rules
$(strip
    $(eval $(SCC_FUZZ)_deps += $(SCC_FUZZ))
    $(foreach __dep,$($(SCC_FUZZ)_deps),
        $(foreach __dir,$(srcdirs),
            $(eval __src := $(__dir)/scc_$(__dep).$(cext))
            $(if $(wildcard $(__src)),
                $(eval
                    $(eval __o := $(patsubst $(__dir)/%.$(cext),$(fuzzbuilddir)/%.$(oext),$(__src)))
                    $(eval obj += $(__o))
                    $(__o): $(__src) | $(fuzzbuilddir)
	                    $$(info [CC] $(notdir $(fuzzbuilddir))/$$(notdir $$@))
	                    $$(CC) $$(CFLAGS) $$(CPPFLAGS) -o $$@ $$^

                    $(fuzztarget): $(__o))))))
endef

$(call fuzz-dependency-rules)
