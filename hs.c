#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include "hs.h"
#include "heap.h"
#include "param.h"
#include "mem.h"

static int hs_node_vec_init(struct hs_node_vec *vec, size_t size)
{
    vec->hs_nodes = (hs_node_t*)hs_calloc(size, sizeof(hs_node_t));
    if(!vec->hs_nodes)
        return -1;
    vec->cap  = size;
    vec->len = 0;
    return 0;
}

static void hs_node_vec_uinit(struct hs_node_vec *vec)
{
    hs_free(vec->hs_nodes);
    vec->cap = 0;
    vec->len = 0;
}

static hs_node_t * hs_node_vec_get(struct hs_node_vec *vec)
{
    if(vec->len < vec->cap)
        return &(vec->hs_nodes[vec->len++]);
    else {
        vec->hs_nodes = hs_realloc(vec->hs_nodes, vec->cap * 2 * sizeof(hs_node_t));
        if(vec->hs_nodes){
            vec->cap *= 2;
            return &(vec->hs_nodes[vec->len ++]);
        }
    }
    return NULL;
}

static hs_node_t *hs_node_vec_at(struct hs_node_vec *vec, unsigned int idx)
{
    return &vec->hs_nodes[idx];
}

static int __uint_compare(
        const void *e1,
        const void *e2,
        const void *udata __attribute__((__unused__))
        )
{
    const unsigned int *i1 = e1;
    const unsigned int *i2 = e2;
    if(*i1 < *i2) return 1;
    if(*i1 > *i2) return -1;
    return 0;
}

int hs_build_aux_init(hs_build_aux_t *aux, int size)
{
    aux->ranges_output = \
                         (struct range1d *)hs_calloc(size*2, sizeof(struct range1d));
    if(!aux->ranges_output) {
        return -1;
    }

    aux->ranges_sort = \
                       (struct range1d *)hs_calloc(size, sizeof(struct range1d));
    if(!aux->ranges_sort) {
        hs_free(aux->ranges_output);
        aux->ranges_output = NULL;
        return -1;
    }

    aux->heap = heap_new(__uint_compare, NULL);
    if(!aux->heap) {
        hs_free(aux->ranges_output);
        hs_free(aux->ranges_sort);
        aux->ranges_output = NULL;
        aux->ranges_sort = NULL;
        return -1;
    }
    return 0;
}

void hs_build_aux_free(hs_build_aux_t *aux)
{
    hs_free(aux->ranges_output);
    hs_free(aux->ranges_sort);
    heap_free(aux->heap);

    aux->ranges_output = NULL;
    aux->ranges_sort = NULL;
    aux->heap = NULL;
}

int hs_build_aux_reinit(hs_build_aux_t *aux, int size)
{
    hs_free(aux->ranges_output);
    hs_free(aux->ranges_sort);

    aux->ranges_output = \
                         (struct range1d *)hs_calloc(size*2, sizeof(struct range1d));
    if(!aux->ranges_output) {
        return -1;
    }

    aux->ranges_sort = \
                       (struct range1d *)hs_calloc(size, sizeof(struct range1d));
    if(!aux->ranges_sort) {
        hs_free(aux->ranges_output);
        aux->ranges_output = NULL;
        return -1;
    }
    return 0;
}

static int hs_prep_build(hs_tree_t *tree, hs_build_aux_t *aux, rule_set_t *ruleset)
{
    int ret;
    tree->ruleset = *ruleset;

    memset(&tree->tree_info, 0, sizeof(tree_info_t));
    ret = hs_node_vec_init(&tree->node_vec, 16);
    if(ret == -1) {
        return -1;
    }

    if(aux == NULL) {
        aux = hs_calloc(1, sizeof(hs_build_aux_t));
        if(!aux) {
            hs_node_vec_uinit(&tree->node_vec);
            return -1;
        }
        ret = hs_build_aux_init(aux, ruleset->num);
        if(ret == -1) {
            hs_free(aux);
            hs_node_vec_uinit(&tree->node_vec);
            return -1;
        }
    }

    tree->aux = aux;
    return 0;
}

static int _range_compare(const void *a, const void *b)
{
    const struct range1d *r1 = (const struct range1d*)a;
    const struct range1d *r2 = (const struct range1d*)b;

    if(r1->low != r2->low)
        if(r1->low < r2->low)
            return -1;
        else
            return 1;
    else if (r1->high != r2->high) {
        if(r1->high < r2->high)
            return -1;
        else
            return 1;
    }
    return 0;
}

static int unique_ranges(struct range1d *ranges, int num)
{
    int i;
    int unique_num = 1;
    for(i = 1; i < num; i ++) {
        if(ranges[i].low != ranges[unique_num -1].low \
                || ranges[i].high != ranges[unique_num -1].high) {
            ranges[unique_num].low = ranges[i].low;
            ranges[unique_num].high = ranges[i].high;
            unique_num ++;
        }
    }
    return unique_num;
}

static unsigned int hs_gen_segs(hs_node_t *node, hs_build_aux_t *aux, int dim)
{
    int i;
    for(i = 0; i < node->ruleset.num; i++) {
        aux->ranges_sort[i].low = node->ruleset.ruleList[i].range[dim][0];
        aux->ranges_sort[i].high = node->ruleset.ruleList[i].range[dim][1];
    }
    qsort(aux->ranges_sort, \
            node->ruleset.num, sizeof(struct range1d), _range_compare);
    int uniq_num = unique_ranges(aux->ranges_sort, node->ruleset.num);

    unsigned long start = aux->ranges_sort[0].low;
    heap_offer(&aux->heap, &(aux->ranges_sort[0].high));

    struct range1d *r;
    struct range1d *rout = aux->ranges_output;
    unsigned long top;
    unsigned int seg_cnt = 0;

    /* because the heap may contain duplicated high value,
     * so each time we have to check if start <= top to form
     * a new segment.
     */

    i = 1;
    for(; i < uniq_num; i ++) {
        r = &aux->ranges_sort[i];
        while(heap_count(aux->heap) != 0 && \
                (top = (unsigned long)(*(unsigned int*)heap_peek(aux->heap))) <= r->low) {
            if(start <= top) {
                rout->low = (unsigned int)start;
                rout->high = (unsigned int)top;
                rout++;
                seg_cnt ++;
                start = top+1;
            }
            heap_poll(aux->heap);
        }
        heap_offer(&aux->heap, &r->high);
        if(start < r->low) {
            rout->low = (unsigned int)start;
            rout->high = r->low -1;
            start = r->low;
            rout++;
            seg_cnt ++;
        }
    }

    while(heap_count(aux->heap) != 0) {
        top = (unsigned long)(*(unsigned int *)heap_peek(aux->heap));
        if(start <= top) {
            rout->low = (unsigned int)start;
            rout->high = (unsigned int)top;
            rout++;
            seg_cnt ++;
            start = top +1;
        }
        heap_poll(aux->heap);
    }
    return seg_cnt;
}

static void remove_redund(rule_set_t *ruleset)
{
    int i,j;

    for(i=1; i < ruleset->num; i++) {
        for(j=0; j < i; j++) {
            if(rule_contained(&ruleset->ruleList[j], \
                        &ruleset->ruleList[i])) {
                ruleset->ruleList[i].pri = UINT32_MAX;
            }
        }
    }

    j = 1;
    for(i=1; i < ruleset->num; i ++) {
        if(ruleset->ruleList[i].pri != UINT32_MAX) {
            ruleset->ruleList[j++] = ruleset->ruleList[i];
        }
    }
    ruleset->num = j;
}


int hs_build(hs_tree_t *tree, unsigned int idx, unsigned int depth)
{
    /* generate segments for input filtset */
    unsigned int	dim, num, seg_cnt;
    int             i,j, ret;
    unsigned int	max_seg_cnt = 1; /* maximum different segment points */
    unsigned int	d2s = 0;		 /* dimension to split (with max diffseg) */
    unsigned int	thresh = 0;
    unsigned int	range[2][2] = {{0, 0}, {0, 0}};     /* sub-space ranges for child-nodes */
    float			hightAvg, hightAll;
    hs_node_t*      currNode = hs_node_vec_at(&tree->node_vec, idx);
    rule_set_t      *ruleset = &currNode->ruleset;

#ifdef	DEBUG
    printf("\n\n>>hs_build at depth=%d", depth);
    printf("\n>>Current Rules:");
    for (num = 0; num < ruleset->num; num++) {
        printf ("\n>>%5dth Rule:", ruleset->ruleList[num].pri);
        for (dim = 0; dim < HS_DIM; dim++) {
            printf (" [%-8x, %-8x]", ruleset->ruleList[num].range[dim][0], ruleset->ruleList[num].range[dim][1]);
        }
    }
#endif /* DEBUG */

    if(ruleset->num <= tree->params.bucketSize) {
        goto LEAF;
    }

    hightAvg = 2*ruleset->num + 1;
    for (dim = 0; dim < HS_DIM; dim ++) {
        seg_cnt = hs_gen_segs(currNode, tree->aux, dim);
        struct range1d *r;
        r = tree->aux->ranges_output;

#ifdef	DEBUG
        printf("\n>>dim[%d] segs: ", dim);
        for (num = 0; num < seg_cnt; num++) {
            printf ("[%u - %u] ", r[num].low, r[num].high);
        }
#endif /* DEBUG */
        if (seg_cnt >= 2) {
            hightAll = 0;
            for (i = 0; i < seg_cnt; i++) {
                r[i].cost = 0;
                for (j = 0; j < ruleset->num; j++) {
                    if (ruleset->ruleList[j].range[dim][0] <= r[i].low \
                            && ruleset->ruleList[j].range[dim][1] >= r[i].high) {
                        r[i].cost ++;
                        hightAll++;
                    }
                }
            }
            if (hightAvg > hightAll/seg_cnt) {	/* possible choice for d2s, pos-1 is the number of segs */
                float hightSum = 0;

                /* select current dimension */
                d2s = dim;
                hightAvg = hightAll/seg_cnt;

                /* the first segment MUST belong to the left child */
                hightSum += r[0].cost;

                if(seg_cnt > 2) {
                    for (num = 1; num < seg_cnt; num++) {
                        thresh = r[num].low - 1;
                        if (hightSum > hightAll/2) {
                            break;
                        }
                        hightSum += r[num].cost;
                    }
                } else {
                    thresh = r[0].high;
                }

                /*printf("\n>>d2s=%u thresh=%x\n", d2s, thresh);*/
                range[0][0] = r[0].low;
                range[0][1] = thresh;
                range[1][0] = thresh + 1;
                range[1][1] = r[seg_cnt-1].high;
            }
            /* print segment list of each dim */
#ifdef	DEBUG
            printf("\n>>hightAvg=%f, hightAll=%f, segs=%d", hightAll/seg_cnt, hightAll, seg_cnt);
            for (num = 0; num < seg_cnt; num++) {
                printf ("\nseg%5d[%8x, %8x](%u)	",
                        num, r[num].low, r[num].high, r[num].cost);
            }
#endif /* DEBUG */
        }
        if(max_seg_cnt < seg_cnt)
            max_seg_cnt = seg_cnt;
    }

    /*Update Leaf node*/
    if (max_seg_cnt <= 1) {
LEAF:
        currNode->depth = depth;
        currNode->child_idx = UINT32_MAX;

        tree->tree_info.NumLeafNode ++;
        tree->tree_info.RulePointers += ruleset->num;

        if (tree->tree_info.WstDepth < depth)
            tree->tree_info.WstDepth = depth;

        if (ruleset->num + depth > tree->tree_info.MaxLeafNum) {
            tree->tree_info.MaxLeafNum = ruleset->num + depth;
            tree->tree_info.MaxL_D = depth;
            tree->tree_info.MaxL_N = ruleset->num;
        }

        if (ruleset->num > tree->tree_info.MaxRulesinLeaf) {
            tree->tree_info.MaxRulesinLeaf = ruleset->num;
        }
        tree->tree_info.AvgDepth += depth;
        return 0;
    }

#ifdef DEBUG
    /* split info */
    printf("\n>>d2s=%u; thresh=0x%8x, range0=[%8x, %8x], range1=[%8x, %8x]",
            d2s, thresh, range[0][0], range[0][1], range[1][0], range[1][1]);
#endif /* DEBUG */

    if (range[1][0] > range[1][1]) {
        printf("\n>>maxDiffSegPts=%d  range[1][0]=%x  range[1][1]=%x",
                max_seg_cnt, range[1][0], range[1][1]);
        /* TODO: this path should be carefully considered */
        return HS_BUILD_INTERNAL_ERR;
    }

    /*Update currNode*/

    tree->tree_info.NumTreeNode ++;
    currNode->d2s = (unsigned char) d2s;
    currNode->depth = (unsigned char) depth;
    currNode->thresh = thresh;

    /* Binary split along d2s*/
    /* allocate both left and right child */
    hs_node_t *child;
    child = hs_node_vec_get(&tree->node_vec);
    if(child == NULL)
        return -1;
    unsigned int child_idx = tree->node_vec.len -1;
    if(hs_node_vec_get(&tree->node_vec) == NULL)
        return -1;

    currNode = hs_node_vec_at(&tree->node_vec, idx);
    ruleset = &currNode->ruleset;

    /*Generate left child rule list*/
    num = 0;
    for (i = 0; i < ruleset->num; i++) {
        if (ruleset->ruleList[i].range[d2s][0] <= range[0][1]
                &&	ruleset->ruleList[i].range[d2s][1] >= range[0][0]) {
            num++;
        }
    }

    child = hs_node_vec_at(&tree->node_vec, child_idx);
    child->ruleset.num = num;
    child->ruleset.ruleList = (rule_t*) hs_malloc( child->ruleset.num * sizeof(rule_t) );
    if(child->ruleset.ruleList == NULL) {
        return -1;
    }

    j = 0;
    for (i = 0; i < ruleset->num; i++) {
        if (ruleset->ruleList[i].range[d2s][0] <= range[0][1]
                &&	ruleset->ruleList[i].range[d2s][1] >= range[0][0]) {
            child->ruleset.ruleList[j] = ruleset->ruleList[i];
            /* in d2s dim, the search space needs to be trimmed off */
            if (child->ruleset.ruleList[j].range[d2s][0] < range[0][0])
                child->ruleset.ruleList[j].range[d2s][0] = range[0][0];
            if (child->ruleset.ruleList[j].range[d2s][1] > range[0][1])
                child->ruleset.ruleList[j].range[d2s][1] = range[0][1];
            j++;
        }
    }
    remove_redund(&child->ruleset);
    ret = hs_build(tree, child_idx, depth+1);
    if(ret < 0) return ret;

    /* hs_build may realloc node_vec, should get currNode again */
    currNode = hs_node_vec_at(&tree->node_vec, idx);
    ruleset = &currNode->ruleset;
    /*Generate right child rule list*/
    num = 0;
    for (i = 0; i < ruleset->num; i++) {
        if (ruleset->ruleList[i].range[d2s][0] <= range[1][1]
                &&	ruleset->ruleList[i].range[d2s][1] >= range[1][0]) {
            num++;
        }
    }

    child = hs_node_vec_at(&tree->node_vec, child_idx + 1);
    child->ruleset.num = num;
    child->ruleset.ruleList = (rule_t*) hs_malloc( child->ruleset.num * sizeof(rule_t) );
    if(child->ruleset.ruleList == NULL) {
        return -1;
    }

    j = 0;
    for (i = 0; i < ruleset->num; i++) {
        if (ruleset->ruleList[i].range[d2s][0] <= range[1][1]
                &&	ruleset->ruleList[i].range[d2s][1] >= range[1][0]) {
            child->ruleset.ruleList[j] = ruleset->ruleList[i];
            /* in d2s dim, the search space needs to be trimmed off */
            if (child->ruleset.ruleList[j].range[d2s][0] < range[1][0])
                child->ruleset.ruleList[j].range[d2s][0] = range[1][0];
            if (child->ruleset.ruleList[j].range[d2s][1] > range[1][1])
                child->ruleset.ruleList[j].range[d2s][1] = range[1][1];
            j++;
        }
    }
    remove_redund(&child->ruleset);
    ret = hs_build(tree, child_idx+1, depth+1);
    if(ret < 0) return ret;

    currNode = hs_node_vec_at(&tree->node_vec, idx);
    currNode->child_idx = child_idx;
    if(idx != 0) {
        /* keep the root rulesets */
        ruleset = &currNode->ruleset;
        hs_free(ruleset->ruleList);
        ruleset->num = 0;
        ruleset->ruleList = NULL;
    }

    return 0;
}

void hs_free_all(hs_tree_t *tree)
{
    int i;
    hs_node_t *n;
    /* the ruleset in the root node is a pointer pointing to the
     * original ruleset, should be freed by hs_acl_ctx_free
     */
    for(i = 1; i < tree->node_vec.len; i++) {
        n = &tree->node_vec.hs_nodes[i];
        if(n->ruleset.num != 0) {
            hs_free(n->ruleset.ruleList);
        }
        n->ruleset.ruleList = NULL;
        n->ruleset.num = 0;
    }

    hs_node_vec_uinit(&tree->node_vec);

    tree->aux = NULL;
    tree->ruleset.num = 0;
    tree->ruleset.ruleList = NULL;
    tree->root = NULL;
}

int hs_build_tree(hs_tree_t *tree, hs_build_aux_t *aux, rule_set_t *ruleset)
{
    int ret = hs_prep_build(tree, aux, ruleset);
    if(ret == -1)
        return -1;

    hs_node_t *root = hs_node_vec_get(&tree->node_vec);
    root->ruleset = *ruleset;

    ret = hs_build(tree, 0, 0);
    if(ret < 0) {
        hs_free_all(tree);
        return ret;
    }
    /* root may change because of realloc in node_vec */
    tree->root = tree->node_vec.hs_nodes;
    return 0;
}

void hs_tree_info(hs_tree_t *tree)
{
    printf("\n\nTreeInfo:");
    printf("\n worst case tree depth:  %d", tree->tree_info.WstDepth);
    printf("\n average tree depth:     %f", (float) tree->tree_info.AvgDepth/tree->tree_info.NumLeafNode);
    printf("\n number of tree nodes:%d", tree->tree_info.NumTreeNode);
    printf("\n number of leaf nodes:%d", tree->tree_info.NumLeafNode);
    printf("\n number of leaf pointers:%d", tree->tree_info.RulePointers);
    printf("\n max memory access %d", tree->tree_info.MaxLeafNum);
    printf("\n max memory access depth %d", tree->tree_info.MaxL_D);
    printf("\n max memory access num %d", tree->tree_info.MaxL_N);
    printf("\n max rules in leaf %d", tree->tree_info.MaxRulesinLeaf);
    unsigned int size = (tree->tree_info.NumTreeNode + tree->tree_info.NumLeafNode) * sizeof(hs_node_t);
    size += tree->tree_info.RulePointers * sizeof(rule_t);
    printf("\n memory size: %d %dK %dM\n", size, size/1024, size/(1024 * 1024));
}

int hs_lookup(hs_tree_t *tree, hs_key_t *hs_key)
{
    hs_node_t *n = tree->root;
    int pri;
    while(n->child_idx != UINT32_MAX) {
        if(hs_key->key[n->d2s] <= n->thresh) {
            n = hs_node_vec_at(&tree->node_vec, n->child_idx);
        } else {
            n = hs_node_vec_at(&tree->node_vec, n->child_idx +1);
        }
    }
    pri = linear_search(&n->ruleset, hs_key);
    return pri;
}

rule_t *hs_lookup_entry(hs_tree_t *tree, hs_key_t *hs_key)
{
    hs_node_t *n = tree->root;
    while(n->child_idx != UINT32_MAX) {
        if(hs_key->key[n->d2s] <= n->thresh) {
            n = hs_node_vec_at(&tree->node_vec, n->child_idx);
        } else {
            n = hs_node_vec_at(&tree->node_vec, n->child_idx +1);
        }
    }
    return linear_search_entry(&n->ruleset, hs_key);
}


