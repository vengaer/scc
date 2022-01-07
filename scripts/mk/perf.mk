bmdir      := $(submoddir)/benchmark
bmbuilddir := $(bmdir)/build
dirs       += $(bmbuilddir)

bmalib     := $(bmbuilddir)/src/libbenchmark.a

bmflags    := -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on

.PHONY: all
all:

$(bmalib): $(machinfo)
$(bmalib): $(call reeval,cpu_cores,$(bmalib))
$(bmalib): $(submodules) | $(bmbuilddir)
	$(CMAKE) -E chdir $(bmbuilddir) $(CMAKE) $(bmflags) $(bmdir)
	$(CMAKE) --build $(bmbuilddir) --parallel $(cpu_cores) --config Release
