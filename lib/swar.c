#include <scc/swar.h>

unsigned char scc_swar_read_byte(scc_vectype vec, unsigned i);
scc_vectype scc_swar_bcast(unsigned char byte);
scc_vectype const *scc_swar_align_load(unsigned char const *ldaddr);
