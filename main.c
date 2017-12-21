#ifndef LIB
#include "hs.h" 
#include "utils-inl.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

char filename[128];
char tracename[128];
int bucketSize = BUCKETSIZE;

void parseargs(int argc, char* argv[])
{
    int c;
    int ok = 0;
    while ((c = getopt(argc, argv, "r:b:t:")) != -1) {
        switch (c) {
            case 'r':
                strncpy(filename, optarg, 128);
                ok = 1;
                break;
            case 'b':
                bucketSize = atoi(optarg);
                break;
            case 't':
                strncpy(tracename, optarg, 128);
                break;
            default:
                break;
        }
    }

    if(ok == 0) {
        printf("wrong args\n");
        exit(-1);
    }
    printf("filename :%s\n", filename);
}

#if __MACH__
#include <mach/mach_time.h>
#define ORWL_NANO (+1.0E-9)
#define ORWL_GIGA UINT64_C(1000000000)

static double orwl_timebase = 0.0;
static uint64_t orwl_timestart = 0;

struct timespec orwl_gettime(void) {
    // be more careful in a multithreaded environement
    if (!orwl_timestart) {
        mach_timebase_info_data_t tb = { 0 };
        mach_timebase_info(&tb);
        orwl_timebase = tb.numer;
        orwl_timebase /= tb.denom;
        orwl_timestart = mach_absolute_time();
    }
    struct timespec t;
    double diff = (mach_absolute_time() - orwl_timestart) * orwl_timebase;
    t.tv_sec = diff * ORWL_NANO;
    t.tv_nsec = diff - (t.tv_sec * ORWL_GIGA);
    return t;
}
#define CLOCK_GETTIME(x) *(x)=orwl_gettime()
#else
#define CLOCK_GETTIME(x) clock_gettime(CLOCK_MONOTONIC, x);
#endif

hs_key_t *sample_rules(rule_set_t *ruleset, int samples_cnt)
{
    hs_key_t *key = calloc(ruleset->num, samples_cnt * sizeof(hs_key_t));
    int i, j, k;
    for(i = 0; i < ruleset->num; i ++) {
        rule_t * r = &(ruleset->ruleList[i]);
        for(j = 0; j < samples_cnt; j ++) {
            for(k = 0; k < DIM; k ++) {
                key[i * samples_cnt + j].key[k] = \
                                                  rand() % ((unsigned long)r->range[k][1] - r->range[k][0] + 1ULL) + r->range[k][0];
            }
        }
    }
    return key;
}

int load_ft(FILE *fpt, hs_key_t *key) 
{
    int ret;
    unsigned int junk, junkmask;
    if(fpt == NULL) 
        return 0;

    ret = fscanf(fpt, "%u\t%u\t%u\t%u\t%u\t%u\t%u\n", &(key->key[0]),
            &(key->key[1]),
            &(key->key[2]),
            &(key->key[3]),
            &(key->key[4]),
            &junk,
            &junkmask);

    if(ret != 7)
        return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    int ret;

    parseargs(argc, argv);
    rule_set_t ruleset;
    ReadFilterFile(&ruleset, filename);

    hs_tree_t tree;
    tree.params.bucketSize = bucketSize;

    struct timespec tp_b;
    struct timespec tp_a;

    CLOCK_GETTIME(&tp_b);
    ret = hs_build_tree(&tree, &ruleset);
    CLOCK_GETTIME(&tp_a);
    printf("Tree Building Time:\n");
    long nano = (tp_a.tv_nsec > tp_b.tv_nsec) ? (tp_a.tv_nsec -tp_b.tv_nsec) : (tp_a.tv_nsec - tp_b.tv_nsec + 1000000000ULL);
    printf("sec %ld, nano %ld\n", tp_b.tv_sec, tp_b.tv_nsec);
    printf("sec %ld, nano %ld\n", tp_a.tv_sec, tp_a.tv_nsec);
    printf("nano %ld us %ld ms %ld\n", nano, nano/1000, nano/1000000UL);

    if(ret == -1) {
        printf("\nBuilding error!\n");
        exit(-1);
    }
    hs_tree_info(&tree);

    //#define SAMPLE
#ifdef SAMPLE
    hs_key_t *keys = sample_rules(&ruleset, 100);

    int i; 
    int pri;

    CLOCK_GETTIME(&tp_b);
    for(i = 0; i < ruleset.num * 100; i ++) {
        pri = hs_lookup(&tree, &keys[i]);
#if 1
        pri ++;
#else
        int lpri = linear_search(&ruleset, &keys[i]);
        if(pri != lpri) {
            printf("tree build error! pri %d lpri %d rule %d\n", pri, lpri, i/100);
        }
#endif
    }
    CLOCK_GETTIME(&tp_a);
    nano = (tp_a.tv_nsec > tp_b.tv_nsec) ? (tp_a.tv_nsec -tp_b.tv_nsec) : (tp_a.tv_nsec - tp_b.tv_nsec + 1000000000ULL);
    printf("sec %ld, nano %ld\n", tp_b.tv_sec, tp_b.tv_nsec);
    printf("sec %ld, nano %ld\n", tp_a.tv_sec, tp_a.tv_nsec);
    printf("nano %ld us %ld ms %ld\n", nano, nano/1000, nano/1000000UL);
    printf("speed %.2fMpps\n", (1e9/((double)nano/(ruleset.num * 100)))/1e6);
#endif

#define TRACE
#ifdef TRACE
    hs_key_t *keys = calloc(1000000, sizeof(hs_key_t));
    int cnt = 0;

    if(tracename[0] != '\0') {
        FILE *fp = fopen(tracename, "r");
        while(load_ft(fp, &keys[cnt])) {
            cnt ++;
        }
        printf("Loading %d traces\n", cnt);

        int i; 
        int pri;
        CLOCK_GETTIME(&tp_b);
        for(i = 0; i < cnt; i ++) {
            pri = hs_lookup(&tree, &keys[i]);
#if 1
            pri ++;
#else
            int lpri = linear_search(&ruleset, &keys[i]);
            if(pri != lpri) {
                printf("tree build error! pri %d lpri %d\n", pri, lpri);
            }
#endif
        }

        CLOCK_GETTIME(&tp_a);
        long nano = (tp_a.tv_nsec > tp_b.tv_nsec) ? (tp_a.tv_nsec -tp_b.tv_nsec) : (tp_a.tv_nsec - tp_b.tv_nsec + 1000000000ULL);
        printf("sec %ld, nano %ld\n", tp_b.tv_sec, tp_b.tv_nsec);
        printf("sec %ld, nano %ld\n", tp_a.tv_sec, tp_a.tv_nsec);
        printf("nano %ld us %ld ms %ld\n", nano, nano/1000, nano/1000000UL);
        printf("speed %.2fMpps\n", (1e9/((double)nano/cnt))/1e6);
    }
#endif
    hs_free_all(&tree);
    return ret;
}
#endif
