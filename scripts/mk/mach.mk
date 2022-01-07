machscript    := $(pyscripts)/mach.$(pyext)
vecscript     := $(pyscripts)/isavec.$(pyext)
machbuilddir  := $(builddir)/mach
dirs          += $(machbuilddir)

# General platform information
machinfo      := $(machbuilddir)/machinfo.$(mkext)
# SIMD ISA information
simdinfo      := $(machbuilddir)/simdinfo.$(mkext)
# Vector information
vecinfo       := $(machbuilddir)/vecinfo.$(mkext)

simdbin       := $(machbuilddir)/simd_isa_detect

__is_cleaning := $(findstring clean,$(MAKECMDGOALS))
__delay       := _

$(machinfo): $(machscript) | $(machbuilddir)
	$(info [PY] $(notdir $@))
	$< -o $@

$(if $(call not,$(__is_cleaning)), \
   $(eval -include $(machinfo))    \
   $(eval -include $(vecinfo)))

as_abi_mach   := $(mach)/$(arch_lower)/$(abi_lower)

__isa_obj     := $(patsubst $(as_abi_mach)/%.$(asext),$(machbuilddir)/%.$(oext),\
                   $(wildcard $(as_abi_mach)/*.$(asext)))

# Force delay of linking of $(simdbin) until $(arch_lower) and $(abi_lower)
# have been set through the above inclusion
__isa_obj     := $(if $(and $(arch_lower)$(abi_lower),$(firstword $(__isa_obj))),$(__isa_obj),$(__delay))

$(simdinfo): $(simdbin)
	$< $@

$(simdbin): $(__isa_obj) | $(machinfo)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(machbuilddir)/%.$(oext): $(as_abi_mach)/%.$(asext) $(machinfo) | $(machbuilddir)
	$(info [AS] $(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS)

$(if $(and $(call not,$(__is_cleaning),$(wildcard $(as_abi_mach)/*.$(asext)))), \
    $(eval -include $(simdinfo)))

$(vecinfo): $(call reeval,simd_isa,$(vecinfo))
$(vecinfo): $(if $(simd_isa),$(simdinfo),$(eval $(vecinfo): simd_isa := UNSUPPORTED))
$(vecinfo): $(vecscript) | $(machbuilddir)
	$(info [PY] $(notdir $@))
	$< -o $@ $(simd_isa)

simd_isa ?= UNSUPPORTED
CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_HOST_OS_$(hostos) -DSCC_ARCH_$(arch) \
            -DSCC_SIMD_ISA_$(simd_isa) -DSCC_VECSIZE=$(vecsize)
