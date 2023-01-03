ifdef __node

$(call push,CPPFLAGS)
$(call push,swsimd_deps)

CPPFLAGS := $(filter-out -DSCC_VECSIZE%,$(CPPFLAGS))
CPPFLAGS += -DSCC_VECSIZE=8
swsimd_deps += hashtab

__node_obj := $(call wildcard-obj,$(srcdir)/swsimd,$(cext))

$(call decl-unit)

$(call pop,swsimd_deps)
$(call pop,CPPFLAGS)

$(__node_builddir)/%.$(oext): $(srcdir)/swsimd/%.$(cext) | $(__node_builddir)
	$(call echo-cc,$(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
