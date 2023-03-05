ifdef __node

__node_autogather :=

x64_simd_isa := $(__node_builddir)/x64_simd_isa

$(x64_simd_isa): $(call wildcard-obj,$(__node_path),$(asext)) | $(__node_builddir)
	$(call echo-ld,$(notdir $@))
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(__node_builddir)/%.$(oext): $(__node_path)/%.$(asext) | $(__node_builddir)
	$(call echo-as,$(notdir $@))
	$(AS) -o $@ $< $(ASFLAGS) $(CPPFLAGS)

$(simdinfo): $(x64_simd_isa)
	$(call echo-gen,$(notdir $@))
	$< $@

else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
