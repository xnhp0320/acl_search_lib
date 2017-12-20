#include <stdlib.h>

__attribute__((weak)) void *hs_calloc(size_t elem, size_t cnt)
{
    return calloc(elem, cnt);
}

__attribute__((weak)) void *hs_realloc(void *orig, size_t cnt)
{
    return realloc(orig, cnt);
}

__attribute__((weak)) void hs_free(void *ptr)
{
    free(ptr);
}

__attribute__((weak)) void *hs_malloc(size_t size)
{
    return malloc(size);
}
