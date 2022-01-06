machscript := $(pyscripts)/mach.$(pyext)
machinfo   := $(builddir)/machinfo.mk

$(machinfo): $(machscript) | $(builddir)
	$< -o $@

-include $(machinfo)

CPPFLAGS += -DSCC_$(bitarch)BIT -DSCC_HOST_OS_$(hostos) -DSCC_ARCH_$(arch)
