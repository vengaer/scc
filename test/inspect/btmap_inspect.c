#include "btmap_inspect.h"

#include <scc/bits.h>
#include <scc/btmap.h>
#include <scc/stack.h>
#include <scc/vec.h>

#include <assert.h>
#include <string.h>

enum { SCC_BTMAP_FLAG_LEAF = 0x01 };

struct nodectx {
    size_t idx;
    void *max;
    void *min;
    struct scc_btmnode_base *node;
};

static inline void *scc_btmnode_keys(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node) {
    return (unsigned char *)node + base->btm_keyoff;
}

static inline struct scc_btmnode_base **scc_btnode_links(struct scc_btmap_base const *restrict base, struct scc_btmnode_base *restrict node) {
    return (void *)((unsigned char *)node + base->btm_linkoff);
}

scc_inspect_mask scc_btmap_impl_inspect_invariants(void const *btmap, size_t keysize) {
    scc_inspect_mask mask = 0u;
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);

    long leafdepth = -1l;
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { 0u, 0, 0, base->btm_root })));

    long depth = 1l;
    struct nodectx *ctx;
    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        unsigned char *keys = scc_btmnode_keys(base, ctx->node);

        if(ctx->max && ctx->idx < ctx->node->btm_nkeys && base->btm_compare(ctx->max, keys + ctx->idx * keysize) < 0) {
            mask |= SCC_BTMAP_ERR_LEFT;
        }
        if(ctx->min && ctx->idx < ctx->node->btm_nkeys && base->btm_compare(ctx->min, keys + ctx->idx * keysize) > 0) {
            mask |= SCC_BTMAP_ERR_RIGHT;
        }

        if(!(ctx->node->btm_flags & SCC_BTMAP_FLAG_LEAF) && ctx->idx <= ctx->node->btm_nkeys) {
            void *min = ctx->idx ? keys + (ctx->idx - 1u) * keysize : ctx->min;
            void *max = ctx->idx == ctx->node->btm_nkeys ? ctx->max : keys + ctx->idx * keysize;

            struct scc_btmnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
            assert(link);

            struct nodectx new = { /* NOLINT(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable) */
                0u, max, min, link
            };

            assert(scc_stack_push(&stack, new));
            ++depth;
        }
        else {
            if(!(ctx->node->btm_flags & SCC_BTMAP_FLAG_LEAF)) {
                if(ctx->node == base->btm_root) {
                    if(ctx->node->btm_nkeys < 1u) {
                        mask |= SCC_BTMAP_ERR_ROOT;
                    }
                }
                else if(ctx->node->btm_nkeys < (base->btm_order >> 1u) - scc_bits_is_even(base->btm_order)) {
                    mask |= SCC_BTMAP_ERR_CHILDREN;
                }
            }
            else {
                if(leafdepth == -1l) {
                    leafdepth = depth;
                }
                else if(depth != leafdepth) {
                    mask |= SCC_BTMAP_ERR_LEAFDEPTH;
                }
            }

            if(ctx->idx++ >= ctx->node->btm_nkeys) {
                scc_stack_pop(stack);
                --depth;
            }
            else {
                ctx->min = keys + (ctx->idx - 1u) * keysize;
            }
        }
    }

    scc_stack_free(stack);
    return mask;
}

void scc_btmap_impl_inspect_dump(void const *restrict btmap, size_t keysize, FILE *fp) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { .idx = 0u, .node = base->btm_root })));

    struct lvlent {
        unsigned lvl;
        scc_vec(unsigned char) bytes;
    };

    scc_vec(struct lvlent) lvls = scc_vec_new(struct lvlent);
    assert(scc_vec_reserve(&lvls, scc_btmap_size(btmap)));

    unsigned lvl = 0u;
    unsigned max_lvl = 0u;
    unsigned total = 0u;

    struct nodectx *ctx;

    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        if(!(ctx->node->btm_flags & SCC_BTMAP_FLAG_LEAF) && ctx->idx <= ctx->node->btm_nkeys) {
            struct scc_btmnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
            assert(link);

            struct nodectx new = { /* NOLINT(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable) */
                .idx = 0u, .node = link
            };

            assert(scc_stack_push(&stack, new));
            ++lvl;
            if(lvl > max_lvl) {
                max_lvl = lvl;
            }
        }
        else if(++ctx->idx >= ctx->node->btm_nkeys) {
                struct lvlent ent = {
                    .lvl = lvl--,
                    .bytes = scc_vec_new_dyn(unsigned char)
                };

                scc_vec_resize(&ent.bytes, ctx->node->btm_nkeys * keysize);
                memcpy(&ent.bytes[0], (unsigned char *)ctx->node + base->btm_keyoff, scc_vec_size(ent.bytes));
                scc_vec_push(&lvls, ent);

                scc_stack_pop(stack);
        }
    }

    (void)fputs("-- begin --\n", fp);
    struct lvlent *iter;
    for(unsigned i = 0u; i <= max_lvl; ++i) {
        (void)fprintf(fp, "%u: ", i);
        scc_vec_foreach(iter, lvls) {
            if(iter->lvl == i) {
                for(unsigned j = 0u; j < scc_vec_size(iter->bytes); ++j) {
                    if(!(j % keysize)) {
                        (void)fputs(" 0x", fp);
                        ++total;
                    }
                    (void)fprintf(fp, "%02x", iter->bytes[j]);
                }
                (void)fputs(" | ", fp);
            }
        }

        (void)fputs("\n\n", fp);
    }

    (void)fprintf(fp, "Total elements: %u\n", total);
    (void)fputs("--  end  --\n", fp);

    scc_vec_foreach(iter, lvls) {
        scc_vec_free(iter->bytes);
    }

    scc_stack_free(stack);
    scc_vec_free(lvls);
}

size_t scc_btmap_inspect_size(void const *btmap) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { .idx = 0u, .node = base->btm_root })));

    size_t total = 0u;

    struct nodectx *ctx;
    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        if(!(ctx->node->btm_flags & SCC_BTMAP_FLAG_LEAF) && ctx->idx <= ctx->node->btm_nkeys) {
            struct scc_btmnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
            assert(link);

            struct nodectx new = { /* NOLINT(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable) */
                .idx = 0u, .node = link
            };

            assert(scc_stack_push(&stack, new));
        }
        else if(++ctx->idx >= ctx->node->btm_nkeys) {
            total += ctx->node->btm_nkeys;
            scc_stack_pop(stack);
        }
    }

    scc_stack_free(stack);
    return total;
}


size_t scc_btmap_impl_inspect_cardinality(void const *restrict btmap, size_t keysize) {
    struct scc_btmap_base const *base = scc_btmap_impl_base_qual(btmap, const);
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { .idx = 0u, .node = base->btm_root })));

    size_t n = 0u;

    struct nodectx *ctx;
    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        if(!(ctx->node->btm_flags & SCC_BTMAP_FLAG_LEAF) && ctx->idx <= ctx->node->btm_nkeys) {
            struct scc_btmnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
            assert(link);

            struct nodectx new = { /* NOLINT(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable) */
                .idx = 0u, .node = link
            };

            assert(scc_stack_push(&stack, new));
        }
        else if(++ctx->idx >= ctx->node->btm_nkeys) {
            unsigned char *keys = (unsigned char *)ctx->node + base->btm_keyoff;
            for(unsigned i = 0u; i < ctx->node->btm_nkeys; ++i) {
                n += !base->btm_compare(btmap, keys + i * keysize);
            }
            scc_stack_pop(stack);
        }
    }

    scc_stack_free(stack);
    return n;
}
