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
# Size of uintmax_t
umaxinfo      := $(machbuilddir)/umaxinfo.$(mkext)

simdbin       := $(machbuilddir)/simd_isa_detect
umaxbin       := $(machbuilddir)/umaxsize
__umaxobj     := $(patsubst $(cscripts)/%.$(cext),$(machbuilddir)/%.$(oext),$(wildcard $(cscripts)/*.$(cext)))

# Assume no simd support
simd_isa      := UNSUPPORTED

__is_cleaning := $(findstring clean,$(MAKECMDGOALS))
__not          = $(if $(1),,_)

$(machinfo): $(machscript) | $(machbuilddir)
	$(info [PY] $(notdir $@))
	$< -o $@

$(if $(call __not,$(__is_cleaning)), \
   $(eval -include $(machinfo))      \
   $(eval -include $(umaxinfo))      \
   $(eval -include $(vecinfo)))

asabiscripts := $(asscripts)/$(arch_lower)/$(abi_lower)

__isa_obj    := $(patsubst $(asabiscripts)/%.$(asext),$(machbuilddir)/%.$(oext),\
                  $(wildcard $(asabiscripts)/*.$(asext)))

# Force delay of linking of $(simdbin) until $(arch_lower) and $(abi_lower)
# have been set through the above inclusion
__isa_obj    := $(if $(arch_lower)$(abi_lower),$(__isa_obj),_)

$(simdinfo): $(simdbin)
	$< $@

$(simdbin): $(__isa_obj) | $(machinfo)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(machbuilddir)/%.$(oext): $(asabiscripts)/%.$(asext) $(machinfo) | $(machbuilddir)
	$(info [AS] $(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS)

$(if $(and $(call __not,$(__is_cleaning), $(wildcard $(asabitscripts)/*.$(asext)))), \
    $(eval -include $(simdinfo)))

$(umaxinfo): $(umaxbin)
	$< $@

$(umaxbin): $(__umaxobj)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(machbuilddir)/%.$(oext): $(cscripts)/%.$(cext) | $(machbuilddir)
	$(info [CC] $(notdir $@))
	$(CC) -o $@ $< $(CFLAGS) $(CPPFLAGS)

# Run only if a supported SIMD ISA has been detected
$(vecinfo): $(vecscript) $(simdinfo) | $(machbuilddir) $(findstring UNSUPPORTED,$(simd_isa))
	$(info [PY] $(notdir $@))
	$< -o $@ $(simd_isa)

vecsize  := $(if $(vecsize),$(vecsize),$(uintmax_t_size))
CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_HOST_OS_$(hostos) -DSCC_ARCH_$(arch) \
            -DSCC_SIMD_ISA_$(simd_isa) -DSCC_VECSIZE=$(vecsize)
