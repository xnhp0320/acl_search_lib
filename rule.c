#include "mem.h"
#include "rule.h"
#include <string.h>

#define HS_ACL_SIZE 201


int hs_acl_ctx_init(hs_acl_ctx_t *ctx, size_t size, int is_v6)
{
    ctx->rs_slice.len = 0;
    ctx->rs_slice.is_v6 = is_v6;
    int rule_size = is_v6 ? sizeof(rule6_t) : sizeof(rule_t);

    ctx->rs_slice.ruleList = hs_calloc(size, rule_size);
    if(!ctx->rs_slice.ruleList) {
        return -1;
    }
    ctx->rs_slice.cap = size;
    return 0;
}

void hs_acl_ctx_free(hs_acl_ctx_t *ctx)
{
    if(ctx->rs_slice.cap != 0) {
        ctx->rs_slice.cap = 0;
        ctx->rs_slice.len = 0;
        hs_free(ctx->rs_slice.ruleList);
        ctx->rs_slice.ruleList = NULL;
    }
}

static int hs_acl_ctx_realloc(hs_acl_ctx_t *ctx, int size)
{
    void *ret;
    int rule_size = RULE_SIZE(&ctx->rs_slice);
    ret = hs_realloc(ctx->rs_slice.ruleList, size * rule_size);

    if(!ret) {
        return -1;
    }

    ctx->rs_slice.ruleList = ret;
    ctx->rs_slice.cap = size;

    return 0;
}

int rule_set_init(hs_acl_ctx_t *ctx, rule_set_t *ruleset)
{
    ruleset->num = ctx->rs_slice.len;
    ruleset->is_v6 = ctx->rs_slice.is_v6;
    ruleset->ruleList = ctx->rs_slice.ruleList;
    return 0;
}

void rule_set_free(rule_set_t *ruleset)
{
    ruleset->num = 0;
    ruleset->ruleList = NULL;
}

/* find the first rule with the index larger than
 * rule->pri, if all the rule.pri is smaller than
 * rule->pri, return rs->len
 */
static int upper_bound(rule_set_slice_t *rs, rule_base_t *rule)
{
    int i;
    i = 0;
    int pos = i;
    int count = rs->len;
    int step;

    while(count > 0) {
        pos = i;
        step = count/2;
        pos += step;
        if(!(rule->pri < rule_base_from_slice(rs, pos)->pri)) {
            i = pos + 1;
            count -= step + 1;
        } else {
            count = step;
        }
    }

    return i;
}

static
int hs_rule_exist_with_pri(hs_acl_ctx_t *ctx, rule_base_t *rule)
{
    int idx = upper_bound(&ctx->rs_slice, rule);
    int i;
    rule_base_t *r;

    if(idx == 0)
        return 0;

    for(i = idx-1; i >= 0 && \
            (r = rule_base_from_slice(&ctx->rs_slice, i))->pri == rule->pri; i --) {
        if(rule_is_equal(r, rule, RS_IS_V6(&ctx->rs_slice))) {
            return 1;
        }
    }
    return 0;
}

int hs_rule_exist(hs_acl_ctx_t *ctx, rule_base_t *rule)
{
    if(rule->pri != UINT32_MAX) {
        return hs_rule_exist_with_pri(ctx, rule);
    }

    int i;
    rule_base_t *r;
    for(i = 0; i < ctx->rs_slice.len; i ++) {
        r = rule_base_from_slice(&ctx->rs_slice, i);
        if(rule_is_equal(r, rule, RS_IS_V6(&ctx->rs_slice))) {
            return 1;
        }
    }
    return 0;
}

int hs_rule_add(hs_acl_ctx_t *ctx, rule_base_t *rule)
{
    int ret = 0;
    if(ctx->rs_slice.len == ctx->rs_slice.cap) {
        ret = hs_acl_ctx_realloc(ctx, ctx->rs_slice.cap * 2);
        if(ret == -1)
            return ret;
    }

    int idx = 0;
    int i;
    rule_base_t *r;
    int is_v6 = RS_IS_V6(&ctx->rs_slice);
    if(ctx->rs_slice.len != 0) {
        idx = upper_bound(&ctx->rs_slice, rule);
        if(idx != ctx->rs_slice.len) {
            if(idx != 0) {
                i = idx -1;
                while(i >= 0 && \
                        (r = rule_base_from_slice(&ctx->rs_slice, i))->pri == rule->pri) {
                    if(rule_is_equal(r, rule, is_v6)) {
                        return HS_RULE_EXIST;
                    }
                    i--;
                }
            }
            /* move [idx, tail] -> [idx+1, tail+1] */
            memmove(rule_base_from_slice(&ctx->rs_slice, idx + 1), \
                    rule_base_from_slice(&ctx->rs_slice, idx), \
                    (ctx->rs_slice.len - idx) * RULE_SIZE(&ctx->rs_slice));
        } else {
            /* all rules' priorities are smaller or equal to
             * the target rules
             */
            i = idx -1;
            while(i >= 0 && \
                    (r = rule_base_from_slice(&ctx->rs_slice, i))->pri == rule->pri) {
                if(rule_is_equal(r, rule, is_v6)) {
                    return HS_RULE_EXIST;
                }
                i--;
            }
        }
    }

    r = rule_base_from_slice(&ctx->rs_slice, idx);
    memcpy(r, rule, RULE_SIZE(&ctx->rs_slice));
    ctx->rs_slice.len ++;
    return 0;
}

int rule_is_equal(rule_base_t *r1, rule_base_t *r2, int is_v6)
{
    int i;
    int count = 0;

    if(r1->pri != UINT32_MAX && r2->pri != UINT32_MAX && \
            r1->pri != r2->pri)
        return 0;

    int dim = RULE_DIM(is_v6);
    for(i = 0; i < dim; i ++) {
        if(r1->range[i][0] == r2->range[i][0] && \
                         r1->range[i][1] == r2->range[i][1])
            count ++;
    }
    return count == dim;
}

static
int hs_rule_find_with_pri(hs_acl_ctx_t *ctx, rule_base_t *rule)
{
    int idx = upper_bound(&ctx->rs_slice, rule);
    if(idx == 0) {
        return HS_RULE_NOT_FOUND;
    }

    int i;
    rule_base_t *r;
    for(i = idx - 1; i >=0 && \
            (r = rule_base_from_slice(&ctx->rs_slice, i))->pri == rule->pri; i--) {
        if(rule_is_equal(r, rule, RS_IS_V6(&ctx->rs_slice))) {
            return i;
        }
    }
    return HS_RULE_NOT_FOUND;
}

int hs_rule_find(hs_acl_ctx_t *ctx, rule_base_t *rule)
{
    if(rule->pri != UINT32_MAX) {
        return hs_rule_find_with_pri(ctx, rule);
    }

    int i;
    for(i = 0; i < ctx->rs_slice.len; i ++) {
        if(rule_is_equal(rule_base_from_slice(&ctx->rs_slice, i), \
                    rule, RS_IS_V6(&ctx->rs_slice))) {
            return i;
        }
    }
    return HS_RULE_NOT_FOUND;
}

int hs_rule_del_with_idx(hs_acl_ctx_t *ctx, int i)
{
    rule_base_t *t = rule_base_from_slice(&ctx->rs_slice, i);
    rule_base_t *s = rule_base_from_slice(&ctx->rs_slice, i + 1);
    memmove(t, s, \
            (ctx->rs_slice.len - i -1) * RULE_SIZE(&ctx->rs_slice));
    ctx->rs_slice.len --;
    return 0;
}

int hs_rule_del(hs_acl_ctx_t *ctx, rule_base_t *rule, hs_del_func delf, void *udata)
{
    int i = hs_rule_find(ctx, rule);
    if(i == HS_RULE_NOT_FOUND) return i;

    if(delf)
        delf(rule_base_from_slice(&ctx->rs_slice, i), udata, RS_IS_V6(&ctx->rs_slice));
    hs_rule_del_with_idx(ctx, i);
    return 0;
}



