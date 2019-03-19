#ifndef __UTILS_INL_H__
#define __UTILS_INL_H__
#include "rule.h"

#define MAXFILTERS	65535 /* support 64K rules */	
struct FILTER						
{
    unsigned int cost;		
#if HS_DIM > HS_DIM6
    unsigned int dim[HS_DIM][2];
#else
    unsigned int dim[HS_DIM6][2];
#endif
    unsigned char act;	
};

struct FILTSET
{
    unsigned int	numFilters;	
    struct FILTER	filtArr[MAXFILTERS];
};


/* read rules from file */
int		ReadFilterFile(rule_set_t* ruleset, char* filename); /* main */
int		ReadFilterFile6(rule_set_t* ruleset, char* filename); /* main */
void	LoadFilters(FILE* fp, struct FILTSET* filtset, int v6);
int		ReadFilter(FILE* fp, struct FILTSET* filtset,	unsigned int cost, int v6);
void	ReadIPRange(FILE* fp, unsigned int* IPrange);
void	ReadIPRange6(FILE* fp, struct FILTSET *flt);
void	ReadPort(FILE* fp, unsigned int* from, unsigned int* to);
void    ReadProtocol(FILE* fp, unsigned int* from, unsigned int* to);

#endif
