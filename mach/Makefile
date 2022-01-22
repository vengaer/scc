ifdef __node

machscript := $(pyscripts)/mach.$(pyext)
vecscript  := $(pyscripts)/isavec.$(pyext)

# General platform information
machinfo   := $(__node_builddir)/machinfo.$(mkext)
# SIMD ISA information
simdinfo   := $(__node_builddir)/isa.$(mkext)
# Vector information
vecinfo    := $(__node_builddir)/vecinfo.$(mkext)

isa_query  := $(__node_builddir)/isa_query

$(machinfo): $(machscript) | $(__node_builddir)
	$(info [PY] $(notdir $@))
	$< -o $@

$(if $(call not,$(__is_cleaning)), \
    $(eval -include $(machinfo))   \
    $(eval -include $(vecinfo)))

query_srcdir := $(__node_path)/$(arch_lower)/$(abi_lower)
__node_obj   := $(call wildcard_obj,$(query_srcdir),$(asext))

# Force delay of linking until $(arch_lower) and $(abi_lower)
# have been set
__node_obj   := $(if $(and $(arch_lower)$(abi_lower),$(firstword $(__node_obj))),$(__node_obj),_)

$(simdinfo): $(isa_query)
	$< $@

$(isa_query): $(__node_obj) | $(__node_builddir)
	$(info [LD] $(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(query_srcdir)/%.$(asext) | $(__node_builddir)
	$(info [AS] $(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS)

$(if $(and $(call not,$(__is_cleaning)),$(wildcard $(query_srcdir)/*.$(asext))), \
    $(eval -include $(simdinfo)))

$(vecinfo): $(call reeval,simd_isa,$(vecinfo))
$(vecinfo): $(if $(simd_isa),$(simdinfo),$(eval $(vecinfo): simd_isa := UNSUPPORTED))
$(vecinfo): $(vecscript) | $(__node_builddir)
	$(info [PY] $(notdir $@))
	$< -o $@ $(simd_isa)

simd_isa ?= UNSUPPORTED
else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory -s
endif
