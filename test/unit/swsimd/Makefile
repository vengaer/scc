ifdef __node

$(call push,CPPFLAGS)

# Force SIMD emulation
CPPFLAGS := $(filter-out -DSCC_VECSIZE%,$(CPPFLAGS))
CPPFLAGS := $(filter-out -DSCC_SIMD_ISA_%,$(CPPFLAGS))
CPPFLAGS += -DSCC_VECSIZE=$(ullsize)

$(call include-node,hashmap_simdemu)
$(call include-node,hashtab_simdemu)

swsimd_deps += hashtab hashmap

$(call pop,CPPFLAGS)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
