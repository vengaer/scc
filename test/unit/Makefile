ifdef __node

runnersuffix    := _runner

$(call push,CFLAGS)
$(call push,CPPFLAGS)
$(call push,LDFLAGS)
$(call push,LDLIBS)

CFLAGS          += -fsanitize=address,undefined
CPPFLAGS        := -I$(unityinc) -I$(testdir) $(filter-out -DNDEBUG,$(CPPFLAGS)) -I$(srcdir)
LDFLAGS         += -fsanitize=address,undefined -L$(unitydir)
LDLIBS          += -lunity -lm

define __unit-obj
$(strip
    $(if $(1),$(eval __uo_subdir := /$(1)))
    $(patsubst $(srcdir)/%.$(cext),$(builddir)/scc$(__uo_subdir)/%.$(oext),
        $(wildcard $(patsubst %,$(srcdir)/%*.$(cext),$(__node) $($(__node)_deps))))
    $(patsubst $(inspectdir)/%.$(cext),$(inspectbuilddir)$(__uo_subdir)/%.$(oext),
        $(wildcard $(inspectdir)/$(__node)*.$(cext)))
    $(patsubst $(asdir)/%.$(asext),$(asbuilddir)/%.$(oext),
        $(wildcard $(patsubst %,$(asdir)/%*.$(asext),$(__node) $($(__node)_deps))))
    $(if $($(__node)_deps),
        $(patsubst $(inspectdir)/%.$(cext),$(inspectbuilddir)$(__uo_subdir)/%.$(oext),
            $(wildcard $(addprefix $(inspectdir)/*,$(addsuffix *.$(cext),$($(__node)_deps))))))
    $(eval __uo_subdir :=))
endef

__mutate-obj = $(call __unit-obj,mutate)

define decl-unit
$(eval
    $(eval __unitsrc   := $(wildcard $(__node_path)/*.$(cext)))
    $(eval __chk       := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/.chk.%.stamp,$(__unitsrc)))
    $(eval __runners   := $(patsubst $(__node_path)/%.$(cext),$(__node_builddir)/%$(runnersuffix),$(__unitsrc)))
    $(eval __unitobj   := $(call __unit-obj))

    # Prevent make from removing generated runners
    .SECONDARY: $(addsuffix .$(cext),$(__runners))

    # Recipies for running tests
    $(foreach __s,$(__chk),
        $(__s): $(call reeval,CPPFLAGS,$(__s))
        $(__s): $(patsubst $(__node_builddir)/.chk.%.stamp,$(__node_builddir)/%$(runnersuffix),$(__s))
	        $$<
	        $(TOUCH) $$@)

    # Runner source generation
    $(__node_builddir)/%$(runnersuffix).$(cext): $(__node_path)/%.$(cext) $(submodules) | $(__node_builddir)
	    $$(call echo-gen,$$(notdir $$@))
	    $(RB) $(unitygenerator) $$< $$@

    $(foreach __r,$(__runners),
        $(__r): $(__r).$(oext) $(patsubst %$(runnersuffix),%.$(oext),$(__r)) $(__unitobj) $(unityalib)
	        $$(call echo-ld,$$(notdir $$@))
	        $(LD) -o $$@ $$^ $(LDFLAGS))

    $(__node_builddir)/%.$(oext): $(__node_path)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    $(__node_builddir)/%.$(oext): $(srcdir)/%.$(cext) | $(__node_builddir)
	    $$(call echo-cc,$$(notdir $$@))
	    $(CC) -o $$@ $$< $(CFLAGS) $(CPPFLAGS)

    check: $(__chk)
)
endef

define decl-mutate
$(eval
    $(call push,CFLAGS)
    $(call push,LDFLAGS)
    $(eval CFLAGS += $(__mull_cflags))
    $(eval LDFLAGS += $(__mull_ldflags))

    $(eval __mut_builddir := $(__node_builddir)/mutation)
    $(eval dirs += $(__mut_builddir))

    $(eval __mutsrc     := $(wildcard $(__node_path)/*.$(cext)))
    $(eval __mut        := $(patsubst $(__node_path)/%.$(cext),$(__mut_builddir)/.mut.%.stamp,$(__mutsrc)))
    $(eval __mutrunners := $(patsubst $(__node_path)/%.$(cext),$(__mut_builddir)/%_mut$(runnersuffix),$(__mutsrc)))
    $(eval __mutobj     := $(call __mutate-obj))

    $(foreach __o,$(__mutobj),
        $(if $(findstring mutate,$(__o)),$(eval __all_obj += $(__o))))

    .SECONDARY: $(addsuffix .$(cext),$(__mutrunners))

    # Running mutation tests
    $(foreach __s,$(__mut),
        $(__s): $(call reeval,CFLAGS,$(__s))
        $(__s): $(call reeval,LDFLAGS,$(__s))
        $(__s): $(call reeval,CPPFLAGS,$(__s))
        $(__s): $(patsubst $(__mut_builddir)/.mut.%.stamp,$(__mut_builddir)/%_mut$(runnersuffix),$(__s)) $(mull_runner)
	        MULL_CONFIG=$(__node_path)/mull.yml $(mull_runner) $$<
	        $(TOUCH) $$@)

    # Generate runner source
    $(__mut_builddir)/%_mut$(runnersuffix).$(cext): $(__node_path)/%.$(cext) $(submodules) | $(__mut_builddir)
	    $$(call echo-gen,$$(notdir $$@))
	    $(RB) $(unitygenerator) $$< $$@

    $(foreach __r,$(__mutrunners),
        $(eval __all_obj += $(__r).$(oext))
        $(__r): $(__r).$(oext) $(patsubst %_mut$(runnersuffix),%.$(oext),$(__r)) $(__mutobj) $(unityalib)
	        $$(call echo-ld,$$(notdir $$@))
	        $(LD) -o $$@ $$^ $(LDLIBS) $$(LDFLAGS)

        $(eval __all_obj += $(__r).$(oext)))

    $(__mut_builddir)/%.$(oext): $(__mut_builddir)/%.$(cext) $(mull_ir_frontend)
	    $$(call echo-cc,$(notdir $(__mut_builddir))/$$(notdir $$@))
	    $(CC) -o $$@ $$< $$(CFLAGS) $$(CPPFLAGS)

    $(__mut_builddir)/%.$(oext): $(__node_path)/%.$(cext) $(mull_ir_frontend) | $(__mut_builddir)
	    $$(call echo-cc,$(notdir $(__mut_builddir))/$$(notdir $$@))
	    $(CC) -o $$@ $$< $$(CFLAGS) $$(CPPFLAGS)

    $(__mut_builddir)/%.$(oext): $(srcdir)/%.$(cext) $(mull_ir_frontend) | $(__mut_builddir)
	    $$(call echo-cc,$(notdir $(__mut_builddir))/$$(notdir  $$@))
	    $(CC) -o $$@ $$< $$(CFLAGS) $$(CPPFLAGS)

    mutate: $(__mut)

    $(call pop,LDFLAGS)
    $(call pop,CFLAGS))
endef

sccbuilddir_mutate     := $(sccbuilddir)/mutate
dirs                   += $(sccbuilddir_mutate)
inspectbuilddir_mutate := $(inspectbuilddir)/mutate
dirs                   += $(inspectbuilddir_mutate)

$(sccbuilddir_mutate)/%.$(oext): $(srcdir)/%.$(cext) $(mull_ir_frontend) | $(sccbuilddir_mutate)
	$(call echo-cc,mutate/$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(__mull_cflags) $(CPPFLAGS)

$(inspectbuilddir_mutate)/%.$(oext): $(inspectdir)/%.$(cext) $(mull_ir_frontend) | $(inspectbuilddir_mutate)
	$(call echo-cc,mutate/$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(__mull_cflags) $(CPPFLAGS)

$(call include-node,algorithm)
$(call include-node,arena)
$(call include-node,bits)
$(call include-node,btmap)
$(call include-node,btree)
$(call include-node,mem)
$(call include-node,hashmap)
$(call include-node,hashtab)
$(call include-node,rbmap)
$(call include-node,rbtree)
$(call include-node,ringdeque)
$(call include-node,stack)
$(call include-node,svec)
$(call include-node,vec)

$(call pop,LDLIBS)
$(call pop,LDFLAGS)
$(call pop,CPPFLAGS)
$(call pop,CFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
