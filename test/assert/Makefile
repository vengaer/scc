ifdef __node

__chk_assert   := $(__node_builddir)/.chk.stamp
asserttestdir  := $(__node_path)

$(__chk_assert): $(__all_mkfiles) $(wildcard $(__node_path)/*.$(pyext)) | $(__node_builddir)
	$(PYTEST) $(PYTESTFLAGS) --root=$(root) $(asserttestdir)
	$(TOUCH) $@

check: $(__chk_assert)

else
# Recurse to top level
__recurse := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),__recurse)
.PHONY: $(__recurse)
$(__recurse):
	@$(MAKE) $(MAKECMDGOALS) -C $(CURDIR)/.. --no-print-directory
endif
