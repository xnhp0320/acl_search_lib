#ifndef _UTILS_H_
#define _UTILS_H_

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
/* ruleset management */
int  rule_set_init(rule_set_t *ruleset);
int  rule_set_add(rule_set_t *ruleset, rule_t *rule);
int  rule_set_del(rule_set_t *ruleset, rule_t *rule);
void rule_set_free(rule_set_t *ruleset);


#endif
