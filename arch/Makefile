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

$(if $(call not,$(__skip_arch)), \
    $(eval -include $(archinfo))   \
    $(eval -include $(vecinfo)))

query_srcdir := $(__node_path)/$(arch_lower)/$(abi_lower)
__node_obj   := $(call wildcard-obj,$(query_srcdir),$(asext))

$(simdinfo): $(isa_query)
	$(call echo-gen,$(notdir $@))
	$< $@

$(if $(arch_lower)$(abi_lower),$(isa_query)): $(__node_obj) | $(archinfo) $(__node_builddir)
	$(call echo-ld,$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(query_srcdir)/%.$(asext) | $(__node_builddir)
	$(call echo-as,$(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS) $(CPPFLAGS)

$(if $(and $(call not,$(__skip_arch)),$(wildcard $(query_srcdir)/*.$(asext))), \
    $(eval -include $(simdinfo)))

$(if $(simd_isa),$(vecinfo)): $(vecscript) $(simdinfo) $(call reeval,simd_isa,$(vecinfo))| $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$< -o $@ $(simd_isa)

simd_isa ?= UNSUPPORTED
CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_SIMD_ISA_$(simd_isa)    \
            -DSCC_HOST_OS_$(hostos) -DSCC_ARCH_$(arch)  -DSCC_VECSIZE=$(vecsize)
else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
