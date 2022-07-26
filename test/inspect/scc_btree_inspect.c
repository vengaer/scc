#include "scc_btree_inspect.h"

#include <scc/scc_btree.h>
#include <scc/scc_stack.h>

#include <assert.h>

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
        }
    }

    scc_stack_free(stack);
    return mask;
}
