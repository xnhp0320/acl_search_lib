#ifndef  _HS_H
#define  _HS_H

#include "param.h"
#include "rule.h"

//#define DEBUG

typedef struct tree_info_s {
    unsigned int    NumTreeNode;
    unsigned int    NumLeafNode;
    unsigned int    WstDepth;
    unsigned int    AvgDepth;
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
    unsigned char       d2s;        /* dimension to split, 2bit is enough */
    unsigned char       depth;      /* tree depth of the node */
    unsigned int        thresh;     /* thresh value to split the current segments */
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
    uint32_t key[0];
} hs_key_t;

typedef struct hs_key4_s {
    uint32_t key[HS_DIM];
} hs_key4_t;

typedef struct hs_key6_s {
    uint32_t key[HS_DIM6];
} hs_key6_t;

/* build hyper-split-tree */
int hs_build(hs_tree_t* ruleset, unsigned int idx, unsigned int depth); /* main */
int hs_build_tree(hs_tree_t *tree, hs_build_aux_t *aux, rule_set_t *ruleset);
void hs_tree_info(hs_tree_t *tree);
int hs_lookup(hs_tree_t *tree, hs_key_t *hs_key);
rule_base_t * hs_lookup_entry(hs_tree_t *tree, hs_key_t *hs_key);
void hs_free_all(hs_tree_t *tree);
int hs_build_aux_init(hs_build_aux_t *aux, int size);
void hs_build_aux_free(hs_build_aux_t *aux);
int hs_build_aux_reinit(hs_build_aux_t *aux, int size);

static inline int hs_rule_count(hs_tree_t *tree)
{
    return tree->ruleset.num;
}

static inline int linear_search(rule_set_t *ruleset, hs_key_t *key)
{
    int i, j;
    rule_base_t *r;
    int dim = RULE_DIM(RS_IS_V6(ruleset));

    for(i=0;i<(int)ruleset->num;i++) {
        r = rule_base_from_rs(ruleset, i);
        for(j = 0; j < dim; j++) {
            if(key->key[j] < r->range[j][0]
                    || key->key[j] > r->range[j][1]) {
                goto next_rule;
            }
        }
        return r->pri;
next_rule:
        continue;
    }

    return -1;
}

static inline rule_base_t * linear_search_entry(rule_set_t *ruleset, hs_key_t *key)
{
    int i, j;
    int dim = RULE_DIM(RS_IS_V6(ruleset));
    rule_base_t *r;

    for(i=0;i<(int)ruleset->num;i++) {
        r = rule_base_from_rs(ruleset, i);
        for (j = 0; j < dim; j++) {
            if(key->key[j] < r->range[j][0]
                    || key->key[j] > r->range[j][1]) {
                goto next_rule;
            }
        }
        return r;
next_rule:
        continue;
    }

    return NULL;
}

static inline int hs_tree_empty(hs_tree_t *tree)
{
    return tree->root == NULL;
}

#define HS_BUILD_INTERNAL_ERR -2

#endif   /* ----- #ifndef _HS_H ----- */
