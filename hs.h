#ifndef  _HS_H
#define  _HS_H

#include "param.h"
#include "rule.h"

//#define DEBUG

typedef struct tree_info_s {
    unsigned int    NumTreeNode;
    unsigned int	NumLeafNode;
    unsigned int	WstDepth;
    unsigned int	AvgDepth;
    unsigned int    MaxLeafNum;
    unsigned int    MaxL_D;
    unsigned int    MaxL_N;
    unsigned int    MaxRulesinLeaf;
    unsigned int    RulePointers;
} tree_info_t;

typedef struct tree_param_s {
    int bucketSize;
} tree_param_t;

#include "heap.h"

struct range1d {
    unsigned int low;
    unsigned int high;
    unsigned int cost;
};

typedef struct hs_node_s
{
    unsigned char		d2s;		/* dimension to split, 2bit is enough */
    unsigned char		depth;		/* tree depth of the node */
    unsigned int		thresh;		/* thresh value to split the current segments */
    unsigned int        child_idx;
    rule_set_t          ruleset;
} hs_node_t;


struct hs_node_vec {
    size_t len;
    size_t cap;
    hs_node_t *hs_nodes;
};

typedef struct hs_build_aux {
    struct range1d * ranges_output;
    struct range1d * ranges_sort;
    heap_t *heap;
} hs_build_aux_t;


typedef struct hs_tree_s {
    hs_build_aux_t *aux;
    struct hs_node_vec node_vec;
    hs_node_t *root;
    tree_info_t tree_info;
    rule_set_t ruleset;
    tree_param_t params;
} hs_tree_t;

typedef struct hs_key_s {
    uint32_t key[DIM];
} hs_key_t;

/* build hyper-split-tree */
int hs_build(hs_tree_t* ruleset, unsigned int idx, unsigned int depth); /* main */
int hs_build_tree(hs_tree_t *tree, hs_build_aux_t *aux, rule_set_t *ruleset);
void hs_tree_info(hs_tree_t *tree);
int hs_lookup(hs_tree_t *tree, hs_key_t *hs_key);
void hs_free_all(hs_tree_t *tree);

static inline int linear_search(rule_set_t *ruleset, hs_key_t *key)
{
    int i;
    for(i=0;i<ruleset->num;i++) {
        if(key->key[0] >= ruleset->ruleList[i].range[0][0] 
                && key->key[0] <= ruleset->ruleList[i].range[0][1]
                && key->key[1] >= ruleset->ruleList[i].range[1][0]
                && key->key[1] <= ruleset->ruleList[i].range[1][1]
                && key->key[2] >= ruleset->ruleList[i].range[2][0]
                && key->key[2] <= ruleset->ruleList[i].range[2][1]
#if DIM == 5 
                && key->key[3] >= ruleset->ruleList[i].range[3][0]
                && key->key[3] <= ruleset->ruleList[i].range[3][1]
                && key->key[4] >= ruleset->ruleList[i].range[4][0]
                && key->key[4] <= ruleset->ruleList[i].range[4][1]
#endif
          ){
            return ruleset->ruleList[i].pri;
        }
    }

    return -1;
}

#endif   /* ----- #ifndef _HS_H ----- */
