#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdint.h>
#include "param.h"


/*-----------------------------------------------------------------------------
 *  structure
 *-----------------------------------------------------------------------------*/
struct FILTER						
{
	unsigned int cost;		
	unsigned int dim[DIM][2];
	unsigned char act;	
};

struct FILTSET
{
	unsigned int	numFilters;	
	struct FILTER	filtArr[MAXFILTERS];
};

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

/* read rules from file */
int		ReadFilterFile(rule_set_t* ruleset, char* filename); /* main */
void	LoadFilters(FILE* fp, struct FILTSET* filtset);
int		ReadFilter(FILE* fp, struct FILTSET* filtset,	unsigned int cost);
void	ReadIPRange(FILE* fp, unsigned int* IPrange);
void	ReadPort(FILE* fp, unsigned int* from, unsigned int* to);
void    ReadProtocol(FILE* fp, unsigned int* from, unsigned int* to);
int rule_contained(rule_t *a, rule_t *b);

/* ruleset management */
int  rule_set_init(rule_set_t *ruleset);
int  rule_set_add(rule_set_t *ruleset, rule_t *rule);
int  rule_set_del(rule_set_t *ruleset, rule_t *rule);
void rule_set_free(rule_set_t *ruleset);


#endif
