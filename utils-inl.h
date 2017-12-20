#ifndef __UTILS_INL_H__
#define __UTILS_INL_H__
#include "utils.h"

#define MAXFILTERS	65535 /* support 64K rules */	
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


/* read rules from file */
int		ReadFilterFile(rule_set_t* ruleset, char* filename); /* main */
void	LoadFilters(FILE* fp, struct FILTSET* filtset);
int		ReadFilter(FILE* fp, struct FILTSET* filtset,	unsigned int cost);
void	ReadIPRange(FILE* fp, unsigned int* IPrange);
void	ReadPort(FILE* fp, unsigned int* from, unsigned int* to);
void    ReadProtocol(FILE* fp, unsigned int* from, unsigned int* to);

#endif
