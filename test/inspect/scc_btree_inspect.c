#include "scc_btree_inspect.h"

#include <scc/scc_btree.h>
#include <scc/scc_stack.h>
#include <scc/scc_svec.h>
#include <scc/scc_vec.h>

#include <assert.h>
#include <string.h>

enum { SCC_BTREE_FLAG_LEAF = 0x01 };

struct nodectx {
    size_t idx;
    void *max;
    void *min;
    struct scc_btnode_base *node;
};

static inline void *scc_btnode_data(struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node) {
    return (unsigned char *)node + base->bt_dataoff;
}

static inline struct scc_btnode_base **scc_btnode_links(struct scc_btree_base const *restrict base, struct scc_btnode_base *restrict node) {
    return (void *)((unsigned char *)node + base->bt_linkoff);
}

scc_inspect_mask scc_btree_impl_inspect_invariants(void const *btree, size_t elemsize) {
    scc_inspect_mask mask = 0u;
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);

    long leafdepth = -1l;
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { 0u, 0, 0, base->bt_root })));

    long depth = 1l;
    struct nodectx *ctx;
    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        unsigned char *data = scc_btnode_data(base, ctx->node);

        if(ctx->max && ctx->idx < ctx->node->bt_nkeys && base->bt_compare(ctx->max, data + ctx->idx * elemsize) < 0) {
            mask |= SCC_BTREE_ERR_LEFT;
        }
        if(ctx->min && ctx->idx < ctx->node->bt_nkeys && base->bt_compare(ctx->min, data + ctx->idx * elemsize) > 0) {
            mask |= SCC_BTREE_ERR_RIGHT;
        }

        if(!(ctx->node->bt_flags & SCC_BTREE_FLAG_LEAF) && ctx->idx <= ctx->node->bt_nkeys) {
            void *min = ctx->idx ? data + (ctx->idx - 1u) * elemsize : ctx->min;
            void *max = ctx->idx == ctx->node->bt_nkeys ? ctx->max : data + ctx->idx * elemsize;

            struct scc_btnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
            assert(link);

            struct nodectx new = { /* NOLINT(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable) */
                0u, max, min, link
            };

            assert(scc_stack_push(&stack, new));
            ++depth;
        }
        else {
            if(!(ctx->node->bt_flags & SCC_BTREE_FLAG_LEAF)) {
                if(ctx->node != base->bt_root && (ctx->min || ctx->max) && ctx->node->bt_nkeys < ((base->bt_order >> 1u) - 1u)) {
                    mask |= SCC_BTREE_ERR_CHILDREN;
                }
            }
            else {
                if(leafdepth == -1l) {
                    leafdepth = depth;
                }
                else if(depth != leafdepth) {
                    mask |= SCC_BTREE_ERR_LEAFDEPTH;
                }
            }

            if(++ctx->idx >= ctx->node->bt_nkeys) {
                scc_stack_pop(stack);
                --depth;
            }
            else {
                ctx->min = data + (ctx->idx - 1u) * elemsize;
            }
        }
    }

    scc_stack_free(stack);
    return mask;
}

void scc_btree_impl_inspect_dump(void const *restrict btree, size_t elemsize, FILE *fp) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { .idx = 0u, .node = base->bt_root })));

    struct lvlent {
        unsigned lvl;
        scc_vec(unsigned char) bytes;
    };

    scc_svec(struct lvlent) lvls = scc_svec_new(struct lvlent);
    assert(scc_svec_reserve(&lvls, scc_btree_size(btree)));

    unsigned lvl = 0u;
    unsigned max_lvl = 0u;
    unsigned total = 0u;

    struct nodectx *ctx;

    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        if(!(ctx->node->bt_flags & SCC_BTREE_FLAG_LEAF) && ctx->idx <= ctx->node->bt_nkeys) {
            struct scc_btnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
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
        else if(++ctx->idx >= ctx->node->bt_nkeys) {
                struct lvlent ent = {
                    .lvl = lvl--,
                    .bytes = scc_vec_new(unsigned char)
                };

                scc_vec_resize(ent.bytes, ctx->node->bt_nkeys * elemsize);
                memcpy(&ent.bytes[0], (unsigned char *)ctx->node + base->bt_dataoff, scc_vec_size(ent.bytes));
                scc_svec_push(&lvls, ent);

                scc_stack_pop(stack);
        }
    }

    (void)fputs("-- begin --\n", fp);
    struct lvlent *iter;
    for(unsigned i = 0u; i <= max_lvl; ++i) {
        (void)fprintf(fp, "%u: ", i);
        scc_svec_foreach(iter, lvls) {
            if(iter->lvl == i) {
                for(unsigned j = 0u; j < scc_vec_size(iter->bytes); ++j) {
                    if(!(j % elemsize)) {
                        (void)fputs(" 0x", fp);
                    }
                    (void)fprintf(fp, "%02x", iter->bytes[j]);
                    ++total;
                }
                (void)fputs(" | ", fp);
            }
        }

        (void)fputs("\n\n", fp);
    }

    fprintf(fp, "Total elements: %u\n", total);
    (void)fputs("--  end  --\n", fp);

    scc_svec_foreach(iter, lvls) {
        scc_vec_free(iter->bytes);
    }

    scc_stack_free(stack);
    scc_svec_free(lvls);
}

size_t scc_btree_inspect_size(void const *btree) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { .idx = 0u, .node = base->bt_root })));

    size_t total = 0u;

    struct nodectx *ctx;
    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        if(!(ctx->node->bt_flags & SCC_BTREE_FLAG_LEAF) && ctx->idx <= ctx->node->bt_nkeys) {
            struct scc_btnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
            assert(link);

            struct nodectx new = { /* NOLINT(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable) */
                .idx = 0u, .node = link
            };

            assert(scc_stack_push(&stack, new));
        }
        else if(++ctx->idx >= ctx->node->bt_nkeys) {
            total += ctx->node->bt_nkeys;
            scc_stack_pop(stack);
        }
    }

    scc_stack_free(stack);
    return total;
}


size_t scc_btree_impl_inspect_cardinality(void const *restrict btree, size_t elemsize) {
    struct scc_btree_base const *base = scc_btree_impl_base_qual(btree, const);
    scc_stack(struct nodectx) stack = scc_stack_new(struct nodectx);
    assert(scc_stack_push(&stack, ((struct nodectx) { .idx = 0u, .node = base->bt_root })));

    size_t n = 0u;

    struct nodectx *ctx;
    while(!scc_stack_empty(stack)) {
        ctx = &scc_stack_top(stack);
        if(!(ctx->node->bt_flags & SCC_BTREE_FLAG_LEAF) && ctx->idx <= ctx->node->bt_nkeys) {
            struct scc_btnode_base *link = scc_btnode_links(base, ctx->node)[ctx->idx++];
            assert(link);

            struct nodectx new = { /* NOLINT(clang-analyzer-deadcode.DeadStores,clang-diagnostic-unused-variable) */
                .idx = 0u, .node = link
            };

            assert(scc_stack_push(&stack, new));
        }
        else if(++ctx->idx >= ctx->node->bt_nkeys) {
            unsigned char *data = (unsigned char *)ctx->node + base->bt_dataoff;
            for(unsigned i = 0u; i < ctx->node->bt_nkeys; ++i) {
                n += !base->bt_compare(btree, data + i * elemsize);
            }
            scc_stack_pop(stack);
        }
    }

    scc_stack_free(stack);
    return n;
}
