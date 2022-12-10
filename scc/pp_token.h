#ifndef SCC_PP_TOKEN_H
#define SCC_PP_TOKEN_H

#define scc_pp_str(x) #x
#define scc_pp_str_expand(x) scc_pp_str(x)

#define scc_pp_cat(x,y) x ## y
#define scc_pp_cat_expand(x,y) scc_pp_cat(x,y)

#define scc_pp_expand(...) __VA_ARGS__

#endif /* SCC_PP_TOKEN_H */
