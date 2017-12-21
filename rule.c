#include "mem.h"
#include "rule.h"
#include <string.h>

#define HS_ACL_SIZE 201 


int hs_acl_ctx_init(hs_acl_ctx_t *ctx)
{
    ctx->rs_slice.len = 0;
    ctx->rs_slice.ruleList = hs_calloc(HS_ACL_SIZE, sizeof(rule_t));
    if(!ctx->rs_slice.ruleList) {
        return -1;
    }
    ctx->rs_slice.cap = HS_ACL_SIZE;
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
    ret = hs_realloc(ctx->rs_slice.ruleList, size * sizeof(rule_t));
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
static int upper_bound(rule_set_slice_t *rs, rule_t *rule)
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
        if(!(rule->pri < rs->ruleList[pos].pri)) {
            i = pos + 1;
            count -= step + 1;
        } else {
            count = step;
        }
    }

    return i;
}

int hs_rule_add(hs_acl_ctx_t *ctx, rule_t *rule)
{
    int ret = 0;
    if(ctx->rs_slice.len == ctx->rs_slice.cap) {
        ret = hs_acl_ctx_realloc(ctx, ctx->rs_slice.cap * 2);
        if(ret == -1)
            return ret;
    }

    int idx = upper_bound(&ctx->rs_slice, rule);
    if(idx != ctx->rs_slice.len) { 
        /* move [idx, tail] -> [idx+1, tail+1] */ 
        memmove(ctx->rs_slice.ruleList + idx, \
                ctx->rs_slice.ruleList + idx + 1,\
                (ctx->rs_slice.len - idx - 1) * sizeof(rule_t));
    }
    ctx->rs_slice.ruleList[idx] = *rule;
    return 0;
}

static int rule_is_equal(rule_t *r1, rule_t *r2)
{
    int i;
    int count = 0;
    for(i = 0; i < DIM; i ++) {
        if(r1->range[i][0] == r2->range[i][0] && \
                         r1->range[i][1] == r2->range[i][1])
            count ++;
    }
    return count == DIM;
}

int hs_rule_del(hs_acl_ctx_t *ctx, rule_t *rule)
{
    int idx = upper_bound(&ctx->rs_slice, rule);
    if(idx == 0 || idx == ctx->rs_slice.len) {
        return HS_RULE_NOT_FOUND;
    }

    int i;
    for(i = idx - 1; i >=0; i--) {
        if(rule_is_equal(&ctx->rs_slice.ruleList[i], \
                            rule)) {
            memmove(&ctx->rs_slice.ruleList[i], \
                    &ctx->rs_slice.ruleList[i+1], 
                    (ctx->rs_slice.len - i -1) *sizeof(rule_t));
            return 0;
        }
    }
    return HS_RULE_NOT_FOUND;
}




