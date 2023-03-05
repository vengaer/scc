ifdef __node

archscript  := $(pyscripts)/arch.$(pyext)
vecscript   := $(pyscripts)/isavec.$(pyext)

# General platform information
archinfo    := $(__node_builddir)/archinfo.$(mkext)
# SIMD ISA information
simdinfo    := $(__node_builddir)/isa.$(mkext)
# Vector information
vecinfo     := $(__node_builddir)/vecinfo.$(mkext)

isa_query   := $(__node_builddir)/isa_query

$(archinfo): $(archscript) | $(__node_builddir)
	$(call echo-gen,$(notdir $@))
	$(PYTHON) $< -o $@

ifneq ($(call not,$(__skip_arch)),)
  -include $(archinfo)
  -include $(simdinfo)
  -include $(vecinfo)
endif

ifneq ($(arch_lower),)
  ifneq ($(wildcard $(__node_path)/$(arch_lower)),)
    $(call include-node,$(arch_lower))
  else
    # TODO any
  endif
endif

ifneq ($(simd_isa),)
  $(vecinfo): $(vecscript) $(simdinfo) $(call reeval,simd_isa,$(vecinfo)) | $(__node_builddir)
	  $(call echo-gen,$(notdir $@))
	  $(PYTHON) $< -o $@ $(simd_isa)
endif

CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_SIMD_ISA_$(simd_isa) -DSCC_HOST_OS_$(hostos)
CPPFLAGS += -DSCC_ARCH_$(arch) -DSCC_VECSIZE=$(if $(__simd_enable),$(vecsize),$(ullsize))
CPPFLAGS += $(if $(__simd_enable),-DSCC_SIMD_ENABLED)

else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
