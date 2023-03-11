ifdef __node

asdir                 := $(__node_path)
simd_isa              := avx2

__avx2_isa_entry      := $(__node_builddir)/.isa.stamp
config_header_entries += $(__avx2_isa_entry)

$(call include-node,lib)

$(__avx2_isa_entry): $(__config_header_init) | $(__node_builddir)
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_SIMD_ISA avx2 -C "SIMD instruction set architecture"
	$(PYTHON) $(__scconfig) $(__config_opts) add SCC_HWVEC_SIZE 32 -C "Size of hardware SIMD vectors"
	$(TOUCH) $@

else

# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
