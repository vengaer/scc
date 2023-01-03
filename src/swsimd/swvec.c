#include "swvec.h"

unsigned char scc_swvec_read_byte(scc_vectype vec, unsigned i);
scc_vectype scc_swvec_bcast(unsigned char byte);
scc_vectype const *scc_swvec_align_load(unsigned char const *ldaddr);
