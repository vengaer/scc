bmdir      := $(submoddir)/benchmark
bmbuilddir := $(bmdir)/build
dirs       += $(bmbuilddir)

bmalib     := $(bmbuilddir)/src/libbenchmark.a
bmmainalib := $(patsubst %.$(aext),%_main.$(aext),$(bmalib))

bmflags    := -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on

.PHONY: all
all:

$(bmalib): $(machinfo)
$(bmalib): $(call reeval,cpu_cores,$(bmalib))

$(bmmainalib): $(machinfo)
$(bmmainalib): $(call reeval,cpu_cores,$(bmalib))

$(bmalib) $(bmmainalib)&: $(submodules) | $(bmbuilddir)
	$(CMAKE) -E chdir $(bmbuilddir) $(CMAKE) $(bmflags) $(bmdir)
	$(CMAKE) --build $(bmbuilddir) --parallel $(cpu_cores) --config Release
