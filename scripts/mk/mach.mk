machscript    := $(pyscripts)/mach.$(pyext)
vecscript     := $(pyscripts)/isavec.$(pyext)
# General platform information
machinfo      := $(builddir)/machinfo.$(mkext)
# SIMD ISA information
simdinfo      := $(builddir)/simdinfo.$(mkext)
# Vector information
vecinfo       := $(builddir)/vecinfo.$(mkext)
# Size of uintmax_t
umaxinfo      := $(builddir)/umaxinfo.$(mkext)

simdbin       := $(builddir)/simd_isa_detect
umaxbin       := $(builddir)/umaxsize
__umaxobj     := $(patsubst $(cscripts)/%.$(cext),$(builddir)/%.$(oext),$(wildcard $(cscripts)/*.$(cext)))

# Assume no simd support
simd_isa      := UNSUPPORTED

__is_cleaning := $(findstring clean,$(MAKECMDGOALS))
__not          = $(if $(1),,_)

$(machinfo): $(machscript) | $(builddir)
	$(info [PY] $(notdir $@))
	$< -o $@

$(if $(call __not,$(__is_cleaning)), \
   $(eval -include $(machinfo))      \
   $(eval -include $(umaxinfo))      \
   $(eval -include $(vecinfo)))

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

$(if $(and $(call __not,$(__is_cleaning), $(wildcard $(asabitscripts)/*.$(asext)))), \
    $(eval -include $(simdinfo)))

$(umaxinfo): $(umaxbin)
	$< $@

$(umaxbin): $(__umaxobj)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(builddir)/%.$(oext): $(cscripts)/%.$(cext) | $(builddir)
	$(info [CC] $(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

# Run only if a supported SIMD ISA has been detected
$(vecinfo): $(vecscript) $(simdinfo) | $(builddir) $(findstring UNSUPPORTED,$(simd_isa))
	$(info [PY] $(notdir $@))
	$< -o $@ $(simd_isa)

vecsize  := $(if $(vecsize),$(vecsize),$(uintmax_t_size))
CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_HOST_OS_$(hostos) -DSCC_ARCH_$(arch) \
            -DSCC_SIMD_ISA_$(simd_isa) -DSCC_VECSIZE=$(vecsize)
