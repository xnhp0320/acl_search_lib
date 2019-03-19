#ifndef __RULE_H__
#define __RULE_H__
/* rule_set_slice_t manange the memory, 
 * ruleset_t is just a user of rule_set_slice_t
 */
#include <stdio.h>
#include <stdint.h>
#include "param.h"

typedef struct rule_base_s
{
    void *          entry;
	unsigned int	pri;
    unsigned int    range[0][2];
} rule_base_t;

typedef	struct rule_s
{
    rule_base_t     base;
	unsigned int	range[HS_DIM][2];
} rule_t;

typedef struct rule6_s
{
    rule_base_t     base;
    unsigned int    range[HS_DIM6][2];
} rule6_t;

typedef struct rule_set_s
{
	unsigned int	num; /* number of rules in the rule set */
    int             is_v6;
    void*			ruleList; /* rules in the set */
} rule_set_t;



/*-----------------------------------------------------------------------------
 *  function declaration
 *-----------------------------------------------------------------------------*/

typedef void (*hs_del_func)(rule_base_t *r, void *userdata, int is_v6);

int rule_contained(rule_base_t *a, rule_base_t *b, int is_v6);
void show_ruleset(rule_set_t *ruleset);

typedef struct rule_set_slice_s
{
    unsigned int cap;
    unsigned int len;
    int is_v6;
    void *ruleList;
} rule_set_slice_t;

typedef struct hs_acl_ctx_s
{
    rule_set_slice_t rs_slice;
} hs_acl_ctx_t;


#define RS_IS_V6(rs) ((rs)->is_v6)
#define RULE_SIZE(rs) ((rs)->is_v6 ? sizeof(rule6_t) : sizeof(rule_t))
#define RULE_DIM(v6) (v6 ? HS_DIM6 : HS_DIM)
#define RULE_IDX(rs, i)  (rule_base_t*)(((rule_t *)rs->ruleList) + idx)
#define RULE6_IDX(rs, i) (rule_base_t*)(((rule6_t *)rs->ruleList) + idx)


static inline rule_base_t *rule_base_from_slice(rule_set_slice_t *slice, int idx)
{
    rule_base_t *base;
    base = slice->is_v6 ? RULE6_IDX(slice, idx) : RULE_IDX(slice, idx);
    return base;
}

static inline rule_base_t *rule_base_from_rs(rule_set_t *rs, int idx)
{
    rule_base_t *base;
    base = rs->is_v6 ? RULE6_IDX(rs, idx) : RULE_IDX(rs, idx);
    return base;
}

int hs_acl_ctx_init(hs_acl_ctx_t *acl_ctx, size_t size, int is_v6);
void hs_acl_ctx_free(hs_acl_ctx_t *acl_ctx);
int  hs_rule_add(hs_acl_ctx_t *ctx, rule_base_t *rule);
int  hs_rule_del(hs_acl_ctx_t *ctx, rule_base_t *rule, hs_del_func delf, void *udata);
int rule_is_equal(rule_base_t *r1, rule_base_t *r2, int is_v6);
int hs_rule_exist(hs_acl_ctx_t *ctx, rule_base_t *rule);
int hs_rule_find(hs_acl_ctx_t *ctx, rule_base_t *rule);
int hs_rule_del_with_idx(hs_acl_ctx_t *ctx, int i);

/* ruleset management */
int  rule_set_init(hs_acl_ctx_t *acl_ctx, rule_set_t *ruleset);
void rule_set_free(rule_set_t *ruleset);

#define HS_RULE_NOT_FOUND -3 
#define HS_RULE_EXIST -2 

#endif
