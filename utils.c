/* functions for ruleset file parsing */
#include <stdlib.h>
#include <string.h>
#include "rule.h"

int _rule_pri_compare(const void *a, const void *b)
{
    const rule_t *r1 = (const rule_t *)a;
    const rule_t *r2 = (const rule_t *)b;
    if(r1->pri < r2->pri)
        return -1;
    else if(r1->pri > r2->pri)
        return 1;
    return 0;
}

void show_ruleset(rule_set_t *ruleset)
{
    int num;
    int dim;
    for (num = 0; num < ruleset->num; num++) {
        printf (">>%5dth Rule:", ruleset->ruleList[num].pri);
        for (dim = 0; dim < HS_DIM; dim++) {
            printf (" [%-8x, %-8x]", ruleset->ruleList[num].range[dim][0], ruleset->ruleList[num].range[dim][1]);
        }
        printf("\n");
    }
}

/* a contains b */
int rule_contained(rule_t *a, rule_t *b)
{
    int i;
    int count = 0;
    for (i = 0; i < HS_DIM; i++) {
        if(a->range[i][0] <= b->range[i][0]
                && a->range[i][1]>= b->range[i][1])
            count ++;
    }
    if(count == HS_DIM)
        return 1;
    else
        return 0;
}

#ifndef LIB
#include "utils-inl.h"
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ReadIPRange
 *  Description:
 * =====================================================================================
 */
void ReadIPRange(FILE* fp, unsigned int* IPrange)
{
    /*asindmemacces IPv4 prefixes*/
    /*temporary variables to store IP range */
    unsigned int trange[4];
    unsigned int mask;
    char validslash;
    int masklit1;
    unsigned int masklit2,masklit3;
    unsigned int ptrange[4];
    int i;
    /*read IP range described by IP/mask*/
    /*fscanf(fp, "%d.%d.%d.%d/%d", &trange[0],&trange[1],&trange[2],&trange[3],&mask);*/
    if (4 != fscanf(fp, "%d.%d.%d.%d", &trange[0],&trange[1],&trange[2],&trange[3])) {
        printf ("\n>> [err] ill-format IP rule-file\n");
        exit (-1);
    }
    if (1 != fscanf(fp, "%c", &validslash)) {
        printf ("\n>> [err] ill-format IP slash rule-file\n");
        exit (-1);
    }
    /*deal with default mask*/
    if(validslash != '/')
        mask = 32;
    else {
        if (1 != fscanf(fp,"%d", &mask)) {
            printf ("\n>> [err] ill-format mask rule-file\n");
            exit (-1);
        }
    }
    mask = 32 - mask;
    masklit1 = mask / 8;
    masklit2 = mask % 8;

    for(i=0;i<4;i++)
        ptrange[i] = trange[i];

    /*count the start IP */
    for(i=3;i>3-masklit1;i--)
        ptrange[i] = 0;
    if(masklit2 != 0){
        masklit3 = 1;
        masklit3 <<= masklit2;
        masklit3 -= 1;
        masklit3 = ~masklit3;
        ptrange[3-masklit1] &= masklit3;
    }
    /*store start IP */
    IPrange[0] = ptrange[0];
    IPrange[0] <<= 8;
    IPrange[0] += ptrange[1];
    IPrange[0] <<= 8;
    IPrange[0] += ptrange[2];
    IPrange[0] <<= 8;
    IPrange[0] += ptrange[3];

    /*count the end IP*/
    for(i=3;i>3-masklit1;i--)
        ptrange[i] = 255;
    if(masklit2 != 0){
        masklit3 = 1;
        masklit3 <<= masklit2;
        masklit3 -= 1;
        ptrange[3-masklit1] |= masklit3;
    }
    /*store end IP*/
    IPrange[1] = ptrange[0];
    IPrange[1] <<= 8;
    IPrange[1] += ptrange[1];
    IPrange[1] <<= 8;
    IPrange[1] += ptrange[2];
    IPrange[1] <<= 8;
    IPrange[1] += ptrange[3];
}

void ReadPort(FILE* fp, unsigned int* from, unsigned int* to)
{
    unsigned int tfrom;
    unsigned int tto;
    if ( 2 !=  fscanf(fp,"%d : %d",&tfrom, &tto)) {
        printf ("\n>> [err] ill-format port range rule-file\n");
        exit (-1);
    }
    *from = tfrom;
    *to = tto;
}

void ReadProtocol(FILE* fp, unsigned int* from, unsigned int* to)
{
    //TODO: currently, only support single protocol, or wildcard
    char dump=0;
    unsigned int proto=0,len=0;
    if ( 7 != fscanf(fp, " %c%c%x%c%c%c%x",&dump,&dump,&proto,&dump,&dump,&dump,&len)) {
        printf ("\n>> [err] ill-format protocol rule-file\n");
        exit (-1);
    }
    if (len==0xff) {
        *from = proto;
        *to = *from;
    } else {
        *from = 0x0;
        *to = 0xff;
    }
}

void ReadPri(FILE *fp, unsigned int *cost)
{
    unsigned int pri;
    if ( 1 != fscanf(fp, "%d",&pri)) {
        printf ("\n>> [err] ill-format protocol rule-file\n");
        exit (-1);
    }
    *cost = pri;
}


int ReadFilter(FILE* fp, struct FILTSET* filtset, unsigned int cost)
{
    /*allocate a few more bytes just to be on the safe side to avoid overflow etc*/
    char validfilter;	/* validfilter means an '@'*/
    struct FILTER *tempfilt,tempfilt1;

    while (!feof(fp))
    {

        if ( 0 != fscanf(fp,"%c",&validfilter)) {
            /*printf ("\n>> [err] ill-format @ rule-file\n");*/
            /*exit (-1);*/
        }
        if (validfilter != '@') continue;	/* each rule should begin with an '@' */

        tempfilt = &tempfilt1;
#if HS_DIM == 5
        ReadIPRange(fp,tempfilt->dim[0]);					/* reading SIP range */
        ReadIPRange(fp,tempfilt->dim[1]);					/* reading DIP range */

        ReadPort(fp,&(tempfilt->dim[2][0]),&(tempfilt->dim[2][1]));
        ReadPort(fp,&(tempfilt->dim[3][0]),&(tempfilt->dim[3][1]));

        ReadProtocol(fp,&(tempfilt->dim[4][0]),&(tempfilt->dim[4][1]));

        /*read action taken by this rule
          fscanf(fp, "%d", &tact);
          tempfilt->act = (unsigned char) tact;

          read the cost (position) , which is specified by the last parameter of this function*/
        tempfilt->cost = cost;
#endif
#if HS_DIM == 3
        ReadIPRange(fp, tempfilt->dim[0]);
        ReadPort(fp,&(tempfilt->dim[1][0]),&(tempfilt->dim[1][1]));
        ReadProtocol(fp,&(tempfilt->dim[2][0]),&(tempfilt->dim[2][1]));
        ReadPri(fp, &cost);
        tempfilt->cost = cost;
#endif
        // copy the temp filter to the global one
        memcpy(&(filtset->filtArr[filtset->numFilters]),tempfilt,sizeof(struct FILTER));

        filtset->numFilters++;
        return 0;
    }
    return -1;
}


void LoadFilters(FILE *fp, struct FILTSET *filtset)
{
    int line = 0;
    filtset->numFilters = 0;
    while(!feof(fp))
    {
        ReadFilter(fp,filtset,line);
        line++;
    }
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:	ReadFilterFile
 *  Description:	Read rules from file.
 *					Rules are stored in 'filterset' for range matching
 * =====================================================================================
 */
int ReadFilterFile(rule_set_t*	ruleset, char* filename)
{
    int		i, j;
    FILE*	fp;
    struct FILTSET	filtset;		/* filter set for range match */


    fp = fopen (filename, "r");
    if (fp == NULL)
    {
        printf("Couldnt open filter set file \n");
        exit(-1);
    }

    LoadFilters(fp, &filtset);
    fclose(fp);

    /*
     *yaxuan: copy rules to dynamic structrue, and from now on, everything is new:-)
     */
    ruleset->num = filtset.numFilters;
    ruleset->ruleList = (rule_t*) malloc(ruleset->num * sizeof(rule_t));
    for (i = 0; i < ruleset->num; i++) {
        ruleset->ruleList[i].pri = filtset.filtArr[i].cost;
        for (j = 0; j < HS_DIM; j++) {
            ruleset->ruleList[i].range[j][0] = filtset.filtArr[i].dim[j][0];
            ruleset->ruleList[i].range[j][1] = filtset.filtArr[i].dim[j][1];
        }
    }
#if HS_DIM == 3
    qsort(ruleset->ruleList, ruleset->num, sizeof(rule_t), _rule_pri_compare);
    show_ruleset(ruleset);
#endif
    /*printf("\n>>number of rules loaded from file: %d", ruleset->num);*/

    return	0;
}
#endif


