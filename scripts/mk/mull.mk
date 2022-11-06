ifndef __Mull_mk
__Mull_mk := _

__mull_cflags  = -fexperimental-new-pass-manager -fpass-plugin=$(mull_ir_frontend) -g -grecord-command-line
__mull_ldflags = -fexperimental-new-pass-manager -fpass-plugin=$(mull_ir_frontend)

endif
