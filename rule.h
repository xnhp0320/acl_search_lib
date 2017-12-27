#ifndef __RULE_H__
#define __RULE_H__
/* rule_set_slice_t manange the memory, 
 * ruleset_t is just a user of rule_set_slice_t
 */
#include <stdio.h>
#include <stdint.h>
#include "param.h"


typedef	struct rule_s
{
	unsigned int	pri;
	unsigned int	range[DIM][2];
} rule_t;

typedef struct rule_set_s
{
	unsigned int	num; /* number of rules in the rule set */
	rule_t*			ruleList; /* rules in the set */
} rule_set_t;



/*-----------------------------------------------------------------------------
 *  function declaration
 *-----------------------------------------------------------------------------*/


int rule_contained(rule_t *a, rule_t *b);

typedef struct rule_set_slice_s
{
    unsigned int cap;
    unsigned int len;
    rule_t       *ruleList;
} rule_set_slice_t;

typedef struct hs_acl_ctx_s
{
    rule_set_slice_t rs_slice;
} hs_acl_ctx_t;

int hs_acl_ctx_init(hs_acl_ctx_t *acl_ctx, size_t size);
void hs_acl_ctx_free(hs_acl_ctx_t *acl_ctx);
int  hs_rule_add(hs_acl_ctx_t *ctx, rule_t *rule);
int  hs_rule_del(hs_acl_ctx_t *ctx, rule_t *rule);

/* ruleset management */
int  rule_set_init(hs_acl_ctx_t *acl_ctx, rule_set_t *ruleset);
void rule_set_free(rule_set_t *ruleset);

#define HS_RULE_NOT_FOUND -1 

#endif
