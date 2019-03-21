#include <assert.h>
#include "rule.h"
#include "hs.h"
#include "param.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void test_1(void)
{
    hs_acl_ctx_t ctx;
    hs_acl_ctx_init(&ctx, 4, 0);
    
    rule_t r1 = {.base = {.pri = 4}, .range = {{0,0}, {0,255}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r2 = {.base = {.pri = 3}, .range = {{0,0}, {0,254}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r3 = {.base = {.pri = 2}, .range = {{0,0}, {0,253}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r4 = {.base = {.pri = 1}, .range = {{0,0}, {0,252}, {0, 255}, {0, 255}, {0, 255}}};

    hs_rule_add(&ctx, (rule_base_t*)&r1);
    hs_rule_add(&ctx, (rule_base_t*)&r2);
    hs_rule_add(&ctx, (rule_base_t*)&r3);
    hs_rule_add(&ctx, (rule_base_t*)&r4);

    assert(rule_base_from_slice(&ctx.rs_slice, 0)->pri == 1);
    assert(rule_base_from_slice(&ctx.rs_slice, 1)->pri == 2);
    assert(rule_base_from_slice(&ctx.rs_slice, 2)->pri == 3);
    assert(rule_base_from_slice(&ctx.rs_slice, 3)->pri == 4);

    hs_acl_ctx_free(&ctx);

}

void test_2(void)
{
    hs_acl_ctx_t ctx;
    hs_acl_ctx_init(&ctx, 4, 0);
    
    rule_t r1 = {.base={.pri = UINT32_MAX}, .range = {{0,0}, {0,255}, {0, 255}, {0, 255}, {0, 255}}};
    int ret;

    ret = hs_rule_del(&ctx, (rule_base_t *)&r1, NULL, NULL);

    assert(ret == HS_RULE_NOT_FOUND);
    hs_acl_ctx_free(&ctx);
}

void test_3(void)
{
    hs_acl_ctx_t ctx;
    hs_acl_ctx_init(&ctx, 4, 0);
    
    rule_t r1 = {.base = {.pri = 4}, .range = {{0,0}, {0,255}, {0, 255}, {0, 255}, {0, 255}}};
    int ret;

    ret = hs_rule_add(&ctx, (rule_base_t *)&r1);
    assert(ret != -1);
    assert(rule_base_from_slice(&ctx.rs_slice, 0)->range[1][1] == 255);
    ret = hs_rule_del(&ctx, (rule_base_t *)&r1, NULL, NULL);
    assert(ret != -1);
    hs_acl_ctx_free(&ctx);
}

void test_4(void)
{
    hs_acl_ctx_t ctx;
    hs_acl_ctx_init(&ctx, 4, 0);
    
    rule_t r1 = {.base={.pri = 2}, .range = {{0,0}, {0,255}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r2 = {.base={.pri = 2}, .range = {{0,0}, {0,254}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r3 = {.base={.pri = 4}, .range = {{0,0}, {0,253}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r4 = {.base={.pri = 3}, .range = {{0,0}, {0,252}, {0, 255}, {0, 255}, {0, 255}}};

    int ret;

    ret = hs_rule_add(&ctx, (rule_base_t *)&r1);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r2);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r3);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r4);

    assert(ret != -1);
    assert(rule_base_from_slice(&ctx.rs_slice, 2)->pri == 3);
    assert(rule_base_from_slice(&ctx.rs_slice, 2)->range[1][1] == 252);
    hs_acl_ctx_free(&ctx);
}

void test_5(void)
{
    hs_acl_ctx_t ctx;
    hs_acl_ctx_init(&ctx, 4, 0);
    
    rule_t r1 = {.base={.pri = 1}, .range = {{0,0}, {0,255}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r2 = {.base={.pri = 1}, .range = {{0,0}, {0,254}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r3 = {.base={.pri = 2}, .range = {{0,0}, {0,253}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r4 = {.base={.pri = 0}, .range = {{0,0}, {0,252}, {0, 255}, {0, 255}, {0, 255}}};

    int ret;

    ret = hs_rule_add(&ctx, (rule_base_t*)&r1);
    ret = hs_rule_add(&ctx, (rule_base_t*)&r2);
    ret = hs_rule_add(&ctx, (rule_base_t*)&r3);
    ret = hs_rule_add(&ctx, (rule_base_t*)&r4);

    assert(ret != -1);
    assert(rule_base_from_slice(&ctx.rs_slice, 0)->pri == 0);
    assert(rule_base_from_slice(&ctx.rs_slice, 0)->range[1][1] == 252);
    hs_acl_ctx_free(&ctx);
}

void test_6(void)
{
    hs_acl_ctx_t ctx;
    hs_acl_ctx_init(&ctx, 1, 0);
    
    rule_t r1 = {.base = {.pri = 1}, .range = {{0,0}, {0,255}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r2 = {.base = {.pri = 1}, .range = {{0,0}, {0,254}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r3 = {.base = {.pri = 2}, .range = {{0,0}, {0,253}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r4 = {.base = {.pri = 0}, .range = {{0,0}, {0,252}, {0, 255}, {0, 255}, {0, 255}}};

    int ret;

    ret = hs_rule_add(&ctx, (rule_base_t *)&r1);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r2);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r3);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r4);

    assert(ret != -1);
    assert(rule_base_from_slice(&ctx.rs_slice, 0)->pri == 0);
    hs_acl_ctx_free(&ctx);
}

void test_7(void)
{
    hs_acl_ctx_t ctx;
    hs_acl_ctx_init(&ctx, 4, 0);
    
    rule_t r1 = {.base = {.pri = 4}, .range = {{0,0}, {0,255}, {0, 255}, {0, 255}, {0, 255}}};
    int ret;

    ret = hs_rule_add(&ctx, (rule_base_t *)&r1);
    assert(ret != -1);

    r1.base.pri = UINT32_MAX;
    ret = hs_rule_del(&ctx, (rule_base_t*)&r1, NULL, NULL);
    assert(ret != -1);
    hs_acl_ctx_free(&ctx);
}

void test_8(void)
{
    hs_acl_ctx_t ctx;
    rule_set_t ruleset;
    int ret;

    hs_acl_ctx_init(&ctx, 4, 0);
    
    rule_t r1 = {.base = {.pri = 1}, .range = {{0,0}, {254,255}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r2 = {.base = {.pri = 2}, .range = {{0,0}, {251,253}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r3 = {.base = {.pri = 3}, .range = {{0,0}, {250,252}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r4 = {.base = {.pri = 4}, .range = {{0,0}, {250,253}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r5 = {.base = {.pri = 5}, .range = {{0,0}, {249,255}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r6 = {.base = {.pri = 6}, .range = {{0,0}, {239,255}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r7 = {.base = {.pri = 7}, .range = {{0,0}, {0,9}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r8 = {.base = {.pri = 8}, .range = {{0,0}, {39,45}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r9 = {.base = {.pri = 9}, .range = {{0,0}, {38,48}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r10 = {.base = {.pri = 10}, .range = {{0,0}, {0,50}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r11 = {.base = {.pri = 11}, .range = {{0,0}, {20,89}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r12 = {.base = {.pri = 12}, .range = {{0,0}, {40,52}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r13 = {.base = {.pri = 13}, .range = {{0,0}, {60,74}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r14 = {.base = {.pri = 14}, .range = {{0,0}, {10,23}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r15 = {.base = {.pri = 15}, .range = {{0,0}, {12,220}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r16 = {.base = {.pri = 16}, .range = {{0,0}, {11,180}, {0, 255}, {0, 255}, {0, 255}}};
    rule_t r17 = {.base = {.pri = 17}, .range = {{0,0}, {100,200}, {0, 255}, {0, 255}, {0, 255}}};

    ret = hs_rule_add(&ctx, (rule_base_t *)&r1);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r2);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r3);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r4);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r5);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r6);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r7);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r8);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r9);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r10);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r11);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r12);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r13);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r14);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r15);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r16);
    ret = hs_rule_add(&ctx, (rule_base_t *)&r17);
    assert(ret != -1);

    rule_set_init(&ctx, &ruleset);
    hs_tree_t tree;
    tree.params.bucketSize = 2;
    ret  = hs_build_tree(&tree, NULL, &ruleset);

    assert(ret != -1);

    hs_key4_t key = {.key = {0, 10, 1, 2, 3}};

    rule_base_t * r = hs_lookup_entry(&tree, (hs_key_t*)&key);
    assert(r != NULL);
    assert(r->pri == 10);
    printf("Test %s: found rule\n", __func__);
    show_rule(r, RS_IS_V6(&ruleset));

    hs_key4_t key1 = {.key = {0, 9, 1, 2, 3}};
    r = hs_lookup_entry(&tree, (hs_key_t*)&key1);
    assert(r != NULL);
    assert(r->pri == 7);
    printf("Test %s: found rule\n", __func__);
    show_rule(r, RS_IS_V6(&ruleset));

    hs_key4_t key2 = {.key = {0, 100, 1, 2, 3}};
    r = hs_lookup_entry(&tree, (hs_key_t*)&key2);
    assert(r != NULL);
    assert(r->pri == 15);
    printf("Test %s: found rule\n", __func__);
    show_rule(r, RS_IS_V6(&ruleset));
    r = linear_search_entry(&ruleset, (hs_key_t*)&key2);
    assert(r != NULL);
    assert(r->pri == 15);
    show_rule(r, RS_IS_V6(&ruleset));

    hs_free_all(&tree);
    hs_acl_ctx_free(&ctx);
}


int main(int argc, char *argv[])
{
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6();
    test_7();
    test_8();
    printf( ANSI_COLOR_GREEN "ALL TEST PASSED\n");
    return 0;
}

