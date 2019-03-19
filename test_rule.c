#include <assert.h>
#include "rule.h"

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

int main(int argc, char *argv[])
{
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6();
    test_7();
    printf("all test passed\n");
    return 0;
}

