ifndef __Mull_mk
__Mull_mk := _

__mull_cflags  = -fexperimental-new-pass-manager -fpass-plugin=$(mull_ir_frontend) -g -grecord-command-line
__mull_ldflags = -fexperimental-new-pass-manager -fpass-plugin=$(mull_ir_frontend)

__ldpaths     := $(addprefix --ld-search-path=,$(wildcard $(addsuffix lib,/ /usr/ /usr/local/)))

MULLFLAGS      = --no-test-output --workers=$(cpu_cores) $(__ldpaths)

endif
