machscript   := $(pyscripts)/mach.$(pyext)
machinfo     := $(builddir)/machinfo.mk
simdinfo     := $(builddir)/simdinfo.mk
simdbin      := $(builddir)/simd_isa_detect
# Assume no simd support
simd_isa     := UNSUPPORTED

$(machinfo): $(machscript) | $(builddir)
	$< -o $@

-include $(machinfo)

asabiscripts := $(asscripts)/$(arch_lower)/$(abi_lower)

__isa_obj    := $(patsubst $(asabiscripts)/%.$(asext),$(builddir)/%.$(oext),\
                  $(wildcard $(asabiscripts)/*.$(asext)))

# Force delay of linking of $(simdbin) until $(arch_lower) and $(abi_lower)
# have been set through the above inclusion
__isa_obj    := $(if $(arch_lower)$(abi_lower),$(__isa_obj),_)

$(simdinfo): $(simdbin)
	$< $@

$(simdbin): $(__isa_obj) | $(machinfo)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(builddir)/%.$(oext): $(asabiscripts)/%.$(asext) $(machinfo) | $(builddir)
	$(info [AS] $(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS)

$(if $(findstring clean,$(MAKECMDGOALS)),        \
    ,                                            \
  $(if $(wildcard $(asabiscripts)/*.$(asext)),   \
      $(eval -include $(simdinfo))))

CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_HOST_OS_$(hostos) -DSCC_ARCH_$(arch) -DSCC_SIMD_ISA_$(simd_isa)

