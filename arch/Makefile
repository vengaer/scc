ifdef __node

archscript := $(pyscripts)/arch.$(pyext)
vecscript  := $(pyscripts)/isavec.$(pyext)

# General platform information
archinfo   := $(__node_builddir)/archinfo.$(mkext)
# SIMD ISA information
simdinfo   := $(__node_builddir)/isa.$(mkext)
# Vector information
vecinfo    := $(__node_builddir)/vecinfo.$(mkext)

isa_query  := $(__node_builddir)/isa_query

$(archinfo): $(archscript) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$< -o $@

$(if $(call not,$(__skip_arch)),                \
    $(call __include-barrier-rel,$(archinfo))   \
    $(call __include-barrier-rel,$(vecinfo)))

query_srcdir := $(__node_path)/$(arch_lower)/$(abi_lower)
__node_obj   := $(call wildcard-obj,$(query_srcdir),$(asext))

$(simdinfo): $(isa_query)
	$(call echo-gen,$(notdir $@))
	$< $@

$(call __include-barrier-acq,$(archinfo),$(isa_query)): $(__node_obj) | $(archinfo) $(__node_builddir)
	$(call echo-ld,$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(query_srcdir)/%.$(asext) | $(__node_builddir)
	$(call echo-as,$(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS) $(CPPFLAGS)

$(if $(and $(call not,$(__skip_arch)),$(wildcard $(query_srcdir)/*.$(asext))), \
    $(call __include-barrier-rel,$(simdinfo)))

$(call __include-barrier-acq,$(simdinfo),$(vecinfo)): $(vecscript) $(simdinfo) $(call reeval,simd_isa,$(vecinfo))| $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$< -o $@ $(simd_isa)

simd_isa ?= UNSUPPORTED
CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_SIMD_ISA_$(simd_isa) -DSCC_HOST_OS_$(hostos)        \
            -DSCC_ARCH_$(arch) -DSCC_VECSIZE=$(if $(__simd_enable),$(vecsize),$(ullsize)) \
            $(if $(__simd_enable),-DSCC_SIMD_ENABLED)
else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
