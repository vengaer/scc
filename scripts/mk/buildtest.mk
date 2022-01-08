buildtestbuilddir := $(builddir)/buildtest
dirs              += $(buildtestbuilddir)
__chk_buildsystem := $(buildtestbuilddir)/.chk.stamp

.PHONY: all
all:

$(__chk_buildsystem): $(wildcard $(mkscripts)/*.$(mkext))
$(__chk_buildsystem): $(wildcard $(pyscripts)/*.$(pyext))
$(__chk_buildsystem): $(call not,$(as_abi_mach))
$(__chk_buildsystem): $(wildcard $(as_abi_mach)/*.$(asext))
$(__chk_buildsystem): Makefile | $(buildtestbuilddir)
	$(PYTEST) $(PYTESTFLAGS) --root=$(root) $(buildtestdir)
	$(TOUCH) $@

check: $(__chk_buildsystem)
